/* Copyright 2015 OpenMarket Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "axolotl/ratchet.hh"
#include "axolotl/message.hh"
#include "axolotl/memory.hh"
#include "axolotl/cipher.hh"

#include <cstring>

namespace {

std::uint8_t PROTOCOL_VERSION = 3;
std::size_t KEY_LENGTH = axolotl::Curve25519PublicKey::LENGTH;
std::uint8_t MESSAGE_KEY_SEED[1] = {0x01};
std::uint8_t CHAIN_KEY_SEED[1] = {0x02};
std::size_t MAX_MESSAGE_GAP = 2000;

void create_chain_key(
    axolotl::SharedKey const & root_key,
    axolotl::Curve25519KeyPair const & our_key,
    axolotl::Curve25519PublicKey const & their_key,
    axolotl::KdfInfo const & info,
    axolotl::SharedKey & new_root_key,
    axolotl::ChainKey & new_chain_key
) {
    axolotl::SharedKey secret;
    axolotl::curve25519_shared_secret(our_key, their_key, secret);
    std::uint8_t derived_secrets[64];
    axolotl::hkdf_sha256(
        secret, sizeof(secret),
        root_key, sizeof(root_key),
        info.ratchet_info, info.ratchet_info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    std::memcpy(new_root_key, derived_secrets, 32);
    std::memcpy(new_chain_key.key, derived_secrets + 32, 32);
    new_chain_key.index = 0;
    axolotl::unset(derived_secrets);
    axolotl::unset(secret);
}


void advance_chain_key(
    axolotl::ChainKey const & chain_key,
    axolotl::ChainKey & new_chain_key
) {
    axolotl::hmac_sha256(
        chain_key.key, sizeof(chain_key.key),
        CHAIN_KEY_SEED, sizeof(CHAIN_KEY_SEED),
        new_chain_key.key
    );
    new_chain_key.index = chain_key.index + 1;
}


void create_message_keys(
    axolotl::ChainKey const & chain_key,
    axolotl::KdfInfo const & info,
    axolotl::MessageKey & message_key
) {
    axolotl::hmac_sha256(
        chain_key.key, sizeof(chain_key.key),
        MESSAGE_KEY_SEED, sizeof(MESSAGE_KEY_SEED),
        message_key.key
    );
    message_key.index = chain_key.index;
}


std::size_t verify_mac_and_decrypt(
    axolotl::Cipher const & cipher,
    axolotl::MessageKey const & message_key,
    axolotl::MessageReader const & reader,
    std::uint8_t * plaintext, std::size_t max_plaintext_length
) {
    return cipher.decrypt(
        message_key.key, sizeof(message_key.key),
        reader.input, reader.input_length,
        reader.ciphertext, reader.ciphertext_length,
        plaintext, max_plaintext_length
    );
}


std::size_t verify_mac_and_decrypt_for_existing_chain(
    axolotl::Ratchet const & session,
    axolotl::ChainKey const & chain,
    axolotl::MessageReader const & reader,
    std::uint8_t * plaintext, std::size_t max_plaintext_length
) {
    if (reader.counter < chain.index) {
        return std::size_t(-1);
    }

    /* Limit the number of hashes we're prepared to compute */
    if (reader.counter - chain.index > MAX_MESSAGE_GAP) {
        return std::size_t(-1);
    }

    axolotl::ChainKey new_chain = chain;

    while (new_chain.index < reader.counter) {
        advance_chain_key(new_chain, new_chain);
    }

    axolotl::MessageKey message_key;
    create_message_keys(new_chain, session.kdf_info, message_key);

    std::size_t result = verify_mac_and_decrypt(
        session.ratchet_cipher, message_key, reader,
        plaintext, max_plaintext_length
    );

    axolotl::unset(new_chain);
    return result;
}


std::size_t verify_mac_and_decrypt_for_new_chain(
    axolotl::Ratchet const & session,
    axolotl::MessageReader const & reader,
    std::uint8_t * plaintext, std::size_t max_plaintext_length
) {
    axolotl::SharedKey new_root_key;
    axolotl::ReceiverChain new_chain;

    /* They shouldn't move to a new chain until we've sent them a message
     * acknowledging the last one */
    if (session.sender_chain.empty()) {
        return false;
    }

    /* Limit the number of hashes we're prepared to compute */
    if (reader.counter > MAX_MESSAGE_GAP) {
        return false;
    }
    std::memcpy(
        new_chain.ratchet_key.public_key, reader.ratchet_key, KEY_LENGTH
    );

    create_chain_key(
        session.root_key, session.sender_chain[0].ratchet_key,
        new_chain.ratchet_key, session.kdf_info,
        new_root_key, new_chain.chain_key
    );

    std::size_t result = verify_mac_and_decrypt_for_existing_chain(
        session, new_chain.chain_key, reader,
        plaintext, max_plaintext_length
    );
    axolotl::unset(new_root_key);
    axolotl::unset(new_chain);
    return result;
}

} // namespace


axolotl::Ratchet::Ratchet(
    axolotl::KdfInfo const & kdf_info,
    Cipher const & ratchet_cipher
) : kdf_info(kdf_info),
    ratchet_cipher(ratchet_cipher),
    last_error(axolotl::ErrorCode::SUCCESS) {
}


void axolotl::Ratchet::initialise_as_bob(
    std::uint8_t const * shared_secret, std::size_t shared_secret_length,
    axolotl::Curve25519PublicKey const & their_ratchet_key
) {
    std::uint8_t derived_secrets[64];
    axolotl::hkdf_sha256(
        shared_secret, shared_secret_length,
        NULL, 0,
        kdf_info.root_info, kdf_info.root_info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    receiver_chains.insert();
    std::memcpy(root_key, derived_secrets, 32);
    std::memcpy(receiver_chains[0].chain_key.key, derived_secrets + 32, 32);
    receiver_chains[0].ratchet_key = their_ratchet_key;
    axolotl::unset(derived_secrets);
}


void axolotl::Ratchet::initialise_as_alice(
    std::uint8_t const * shared_secret, std::size_t shared_secret_length,
    axolotl::Curve25519KeyPair const & our_ratchet_key
) {
    std::uint8_t derived_secrets[64];
    axolotl::hkdf_sha256(
        shared_secret, shared_secret_length,
        NULL, 0,
        kdf_info.root_info, kdf_info.root_info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    sender_chain.insert();
    std::memcpy(root_key, derived_secrets, 32);
    std::memcpy(sender_chain[0].chain_key.key, derived_secrets + 32, 32);
    sender_chain[0].ratchet_key = our_ratchet_key;
    axolotl::unset(derived_secrets);
}


std::size_t axolotl::Ratchet::pickle_max_output_length() {
    std::size_t counter_length = 4;
    std::size_t send_chain_length = counter_length + 64 + 32;
    std::size_t recv_chain_length = counter_length + 32 + 32;
    std::size_t skip_key_length = counter_length + 32 + 32 + 32 + 16;
    std::size_t pickle_length = 3 * counter_length + 32;
    pickle_length += sender_chain.size() * send_chain_length;
    pickle_length += receiver_chains.size() * recv_chain_length;
    pickle_length += skipped_message_keys.size() * skip_key_length;
    return pickle_length;
}

namespace {

std::uint8_t * pickle_counter(
    std::uint8_t * output, std::uint32_t value
) {
    unsigned i = 4;
    output += 4;
    while (i--) { *(--output) = value; value >>= 8; }
    return output + 4;
}

std::uint8_t * unpickle_counter(
    std::uint8_t *input, std::uint32_t &value
) {
    unsigned i = 4;
    value = 0;
    while (i--) { value <<= 8; value |= *(input++); }
    return input;
}

std::uint8_t * pickle_bytes(
    std::uint8_t * output, std::size_t count, std::uint8_t const * bytes
) {
    std::memcpy(output, bytes, count);
    return output + count;
}

std::uint8_t * unpickle_bytes(
    std::uint8_t * input, std::size_t count, std::uint8_t * bytes
) {
    std::memcpy(bytes, input, count);
    return input + count;
}

} // namespace


std::size_t axolotl::Ratchet::pickle(
    std::uint8_t * output, std::size_t max_output_length
) {
    std::uint8_t * pos = output;
    if (max_output_length < pickle_max_output_length()) {
        last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    pos = pickle_counter(pos, sender_chain.size());
    pos = pickle_counter(pos, receiver_chains.size());
    pos = pickle_counter(pos, skipped_message_keys.size());
    pos = pickle_bytes(pos, 32, root_key);
    for (const axolotl::SenderChain &chain : sender_chain) {
        pos = pickle_counter(pos, chain.chain_key.index);
        pos = pickle_bytes(pos, 32, chain.chain_key.key);
        pos = pickle_bytes(pos, 32, chain.ratchet_key.public_key);
        pos = pickle_bytes(pos, 32, chain.ratchet_key.private_key);
    }
    for (const axolotl::ReceiverChain &chain : receiver_chains) {
        pos = pickle_counter(pos, chain.chain_key.index);
        pos = pickle_bytes(pos, 32, chain.chain_key.key);
        pos = pickle_bytes(pos, 32, chain.ratchet_key.public_key);
    }
    for (const axolotl::SkippedMessageKey &key : skipped_message_keys) {
        pos = pickle_counter(pos, key.message_key.index);
        pos = pickle_bytes(pos, 32, key.message_key.key);
        pos = pickle_bytes(pos, 32, key.ratchet_key.public_key);
    }
    return pos - output;
}

std::size_t axolotl::Ratchet::unpickle(
    std::uint8_t * input, std::size_t input_length
) {

    std::uint8_t * pos = input;
    std::uint8_t * end = input + input_length;
    std::uint32_t send_chain_num, recv_chain_num, skipped_num;

    if (end - pos < 4 * 3 + 32) {} // input too small.

    pos = unpickle_counter(pos, send_chain_num);
    pos = unpickle_counter(pos, recv_chain_num);
    pos = unpickle_counter(pos, skipped_num);
    pos = unpickle_bytes(pos, 32, root_key);

    if (end - pos < send_chain_num * (32 * 3 + 4)) {} // input too small.

    while (send_chain_num--) {
        axolotl::SenderChain & chain = *sender_chain.insert(
            sender_chain.end()
        );
        pos = unpickle_counter(pos, chain.chain_key.index);
        pos = unpickle_bytes(pos, 32, chain.chain_key.key);
        pos = unpickle_bytes(pos, 32, chain.ratchet_key.public_key);
        pos = unpickle_bytes(pos, 32, chain.ratchet_key.private_key);
    }

    if (end - pos < recv_chain_num * (32 * 2 + 4)) {} // input too small.

    while (recv_chain_num--) {
        axolotl::ReceiverChain & chain = *receiver_chains.insert(
            receiver_chains.end()
        );
        pos = unpickle_counter(pos, chain.chain_key.index);
        pos = unpickle_bytes(pos, 32, chain.chain_key.key);
        pos = unpickle_bytes(pos, 32, chain.ratchet_key.public_key);
    }

    if (end - pos < skipped_num * (32 * 3 + 16 + 4)) {} // input too small.

    while (skipped_num--) {
        axolotl::SkippedMessageKey &key = *skipped_message_keys.insert(
            skipped_message_keys.end()
        );
        pos = unpickle_counter(pos, key.message_key.index);
        pos = unpickle_bytes(pos, 32, key.message_key.key);
        pos = unpickle_bytes(pos, 32, key.ratchet_key.public_key);
    }
    return pos - input;
}


std::size_t axolotl::Ratchet::encrypt_max_output_length(
    std::size_t plaintext_length
) {
    std::size_t counter = 0;
    if (!sender_chain.empty()) {
        counter = sender_chain[0].chain_key.index;
    }
    std::size_t padded = ratchet_cipher.encrypt_ciphertext_length(
        plaintext_length
    );
    return axolotl::encode_message_length(
        counter, KEY_LENGTH, padded, ratchet_cipher.mac_length()
    );
}


std::size_t axolotl::Ratchet::encrypt_random_length() {
    return sender_chain.empty() ? KEY_LENGTH : 0;
}


std::size_t axolotl::Ratchet::encrypt(
    std::uint8_t const * plaintext, std::size_t plaintext_length,
    std::uint8_t const * random, std::size_t random_length,
    std::uint8_t * output, std::size_t max_output_length
) {
    std::size_t output_length = encrypt_max_output_length(plaintext_length);

    if (random_length < encrypt_random_length()) {
        last_error = axolotl::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }
    if (max_output_length < output_length) {
        last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    if (sender_chain.empty()) {
        sender_chain.insert();
        axolotl::generate_key(random, sender_chain[0].ratchet_key);
        create_chain_key(
            root_key,
            sender_chain[0].ratchet_key,
            receiver_chains[0].ratchet_key,
            kdf_info,
            root_key, sender_chain[0].chain_key
        );
    }

    MessageKey keys;
    create_message_keys(sender_chain[0].chain_key, kdf_info, keys);
    advance_chain_key(sender_chain[0].chain_key, sender_chain[0].chain_key);

    std::size_t ciphertext_length = ratchet_cipher.encrypt_ciphertext_length(
        plaintext_length
    );
    std::uint32_t counter = keys.index;
    Curve25519PublicKey const & ratchet_key = sender_chain[0].ratchet_key;

    axolotl::MessageWriter writer;

    axolotl::encode_message(
        writer, PROTOCOL_VERSION, counter, KEY_LENGTH, ciphertext_length, output
    );

    std::memcpy(writer.ratchet_key, ratchet_key.public_key, KEY_LENGTH);

    ratchet_cipher.encrypt(
        keys.key, sizeof(keys.key),
        plaintext, plaintext_length,
        writer.ciphertext, ciphertext_length,
        output, output_length
    );

    axolotl::unset(keys);
    return output_length;
}


std::size_t axolotl::Ratchet::decrypt_max_plaintext_length(
    std::size_t input_length
) {
    return input_length;
}


std::size_t axolotl::Ratchet::decrypt(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * plaintext, std::size_t max_plaintext_length
) {
    if (max_plaintext_length < decrypt_max_plaintext_length(input_length)) {
        last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    axolotl::MessageReader reader;
    axolotl::decode_message(
        reader, input, input_length, ratchet_cipher.mac_length()
    );

    if (reader.version != PROTOCOL_VERSION) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_VERSION;
        return std::size_t(-1);
    }

    if (!reader.has_counter || !reader.ratchet_key || !reader.ciphertext) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
        return std::size_t(-1);
    }

    if (reader.ratchet_key_length != KEY_LENGTH) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
        return std::size_t(-1);
    }

    ReceiverChain * chain = NULL;
    for (axolotl::ReceiverChain & receiver_chain : receiver_chains) {
        if (0 == std::memcmp(
                receiver_chain.ratchet_key.public_key, reader.ratchet_key,
                KEY_LENGTH
        )) {
            chain = &receiver_chain;
            break;
        }
    }

    std::size_t result = std::size_t(-1);

    if (!chain) {
        result = verify_mac_and_decrypt_for_new_chain(
            *this, reader, plaintext, max_plaintext_length
        );
    } else if (chain->chain_key.index > reader.counter) {
        /* Chain already advanced beyond the key for this message
         * Check if the message keys are in the skipped key list. */
        for (axolotl::SkippedMessageKey & skipped : skipped_message_keys) {
            if (reader.counter == skipped.message_key.index
                    && 0 == std::memcmp(
                        skipped.ratchet_key.public_key, reader.ratchet_key,
                        KEY_LENGTH
                    )
            ) {
                /* Found the key for this message. Check the MAC. */

                result = verify_mac_and_decrypt(
                    ratchet_cipher, skipped.message_key, reader,
                    plaintext, max_plaintext_length
                );

                if (result != std::size_t(-1)) {
                    /* Remove the key from the skipped keys now that we've
                     * decoded the message it corresponds to. */
                    axolotl::unset(skipped);
                    skipped_message_keys.erase(&skipped);
                    return result;
                }
            }
        }
    } else {
        result = verify_mac_and_decrypt_for_existing_chain(
            *this, chain->chain_key, reader, plaintext, max_plaintext_length
        );
    }

    if (result == std::size_t(-1)) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_MAC;
        return std::size_t(-1);
    }

    if (!chain) {
        /* They have started using a new empheral ratchet key.
         * We need to derive a new set of chain keys.
         * We can discard our previous empheral ratchet key.
         * We will generate a new key when we send the next message. */
        chain = receiver_chains.insert();
        std::memcpy(
            chain->ratchet_key.public_key, reader.ratchet_key, KEY_LENGTH
        );
        create_chain_key(
            root_key, sender_chain[0].ratchet_key, chain->ratchet_key,
            kdf_info, root_key, chain->chain_key
        );
        axolotl::unset(sender_chain[0]);
        sender_chain.erase(sender_chain.begin());
    }

    while (chain->chain_key.index < reader.counter) {
        axolotl::SkippedMessageKey & key = *skipped_message_keys.insert();
        create_message_keys(chain->chain_key, kdf_info, key.message_key);
        key.ratchet_key = chain->ratchet_key;
        advance_chain_key(chain->chain_key, chain->chain_key);
    }

    advance_chain_key(chain->chain_key, chain->chain_key);

    return result;
}
