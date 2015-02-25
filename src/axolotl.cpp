#include "axolotl/axolotl.hh"


namespace {

std::uint8_t PROTOCOL_VERSION = 3;
std::size_t MAC_LENGTH = 8;
std::size_t KEY_LENGTH = Curve25519PublicKey::Length;
std::uint8_t MESSAGE_KEY_SEED[1] = {0x01};
std::uint8_t CHAIN_KEY_SEED[1] = {0x02};
std::size_t MAX_MESSAGE_GAP = 2000;

void create_chain_key(
    axolotl::SharedKey const & root_key,
    Curve25519KeyPair const & our_key,
    Curve25519PublicKey const & their_key,
    std::uint8_t const * info, std::size_t info_length,
    SharedSecret & new_root_key,
    ChainKey & new_chain_key
) {
    axolotl::SharedSecret secret;
    axolotl::curve25519_shared_secret(our_key, their_key, secret);
    std::uint8_t derived_secrets[64];
    axolotl::hkdf_sha256(
        secret, sizeof(secret),
        root_key, sizeof(root_key),
        info, info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    std::memcpy(new_root_key, derived_secrets, 32);
    std::memcpy(new_chain_key.key, derived_secrets + 32, 32);
    new_chain_key.index = 0;
    std::memset(derived_secrets, 0, sizeof(derived_secrets);
    std::memset(secret, 0, sizeof(secret));
}


void advance_chain_key(
    ChainKey const & chain_key,
    ChainKey & new_chain_key,
) {
    axolotl::hmac_sha256(
        chain_key.key, sizeof(chain_key.key),
        CHAIN_KEY_SEED, sizeof(CHAIN_KEY_SEED),
        new_chain_key.key
    );
    new_chain_key.index = chain_key.index + 1;
}


void create_message_keys(
    ChainKey const & chain_key,
    std::uint8_t const * info, std::size_t info_length,
    MessageKey & message_key
) {
    axolotl::SharedSecret secret;
    axolotl::hmac_sha256(
        chain_key.key, sizeof(chain_key.key),
        MESSAGE_KEY_SEED, sizeof(MESSAGE_KEY_SEED),
        secret
    );
    std::uint8_t derived_secrets[80];
    axolotl::hkdf_sha256(
        secret, sizeof(secret),
        root_key, sizeof(root_key),
        info, info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    std::memcpy(message_key.cipher_key, derived_secrets, 32);
    std::memcpy(message_key.mac_key, derived_secrets + 32, 32);
    std::memcpy(message_key.iv, derived_secrets + 64, 16);
    message_key.index = chain_key.index;
    std::memset(derived_secrets, 0, sizeof(derived_secrets);
    std::memset(secret, 0, sizeof(secret));
}


bool verify_mac(
    MessageKey const & message_key,
    std::uint8_t const * input,
    axolotl::MessageReader const & reader
) {
    std::uint8_t mac[HMAC_SHA256_OUTPUT_LENGTH];
    axolotl::hmac_sha256(
        keys.mac_key, sizeof(keys.mac_key),
        ciphertext, reader.body_length,
        mac
    );

    bool result = std::memcmp(mac, reader.mac, MAC_LENGTH) == 0;
    std::memset(&mac, 0, HMAC_SHA256_OUTPUT_LENGTH);
    return result;
}


bool verify_mac_for_existing_chain(
    axolotl::Session const & session,
    axolotl::ReceiverChain const & chain,
    std::uint8_t const * input,
    axolotl::MessageReader const & reader
) {
    ReceiverChain new_chain = chain;

    if (reader.counter < chain.index) {
        return false;
    }

    /* Limit the number of hashes we're prepared to compute */
    if (reader.counter - chain.index > MAX_MESSAGE_GAP) {
        return false;
    }

    while (new_chain.index < reader.counter) {
        advance_chain_key(new_chain, new_chain);
    }

    MessageKey message_key;
    create_message_keys(
        new_chain_key, sender.message_info, sender.message_info_length,
        message_key
    );

    bool result = verify_mac(message_key, input, reader);
    std::memset(&new_chain, 0, sizeof(new_chain.ratchet_key);
    return result;
}


bool verify_mac_for_new_chain(
    axolotl::Session const & session,
    std::uint8_t const * input,
    axolotl::MessageReader const & reader
) {
    SharedSecret new_root_key;
    ReceiverChain new_chain;

    /* They shouldn't move to a new chain until we've sent them a message
     * acknowledging the last one */
    if (session.sender_chain.empty()) {
        return false;
    }

    /* Limit the number of hashes we're prepared to compute */
    if (reader.counter > MAX_MESSAGE_GAP) {
        return false;
    }
    std::memcpy(new_chain.ratchet_key, reader.ratchet_key, KEY_LENGTH);

    create_chain_key(
        root_key, sender_chain[0].ratchet_key, new_chain.ratchet_key,
        session.kdf_info.ratchet_info, session.kdf_info.ratchet_info_length,
        new_root_key, new_chain
    );

    bool result = verify_mac_for_existing_chain(
        session, new_chain, input, reader
    );
    std::memset(&new_root_key, 0, sizeof(new_root_key));
    std::memset(&new_chain, 0, sizeof(new_chain.ratchet_key);
    return result;
}

} // namespace


std::size_t axolotl::Session::encrypt_max_output_length(
    std::size_t plaintext_length
) {
    std::size_t key_length = 1 + varstring_length(Curve25519PublicKey::Length);
    std::size_t counter = sender_chain.empty() ? 0 : sender_chain[0].index;
    std::size_t padded = axolotl::aes_encrypt_cbc_length(plaintext_length);
    return axolotl::encode_message_length(
        counter, KEY_LENGTH, padded, MAC_LENGTH
    );
}


std::size_t axolotl::Session::encrypt_random_length() {
    return sender_chain.size() ? Curve25519PublicKey::Length : 0;
}


std::size_t axolotl::Session::encrypt(
    std::uint8_t const * plaintext, std::size_t plaintext_length,
    std::uint8_t const * random, std::size_t random_length,
    std::uint8_t * output, std::size_t max_output_length
) {
    if (random_length < encrypt_random_length()) {
        last_error = axolotl::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }
    if (max_output_length < encrypt_max_output_length()) {
        last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    if (sender_chain.empty()) {
        /** create sender chain */
    }

    MessageKey keys;

    /** create message keys and advance chain */

    std::size_t padded = axolotl::aes_encrypt_cbc_length(plaintext_length);
    std::size_t key_length = Curve25519PublicKey::Length;
    std::uint32_t counter = keys.index;
    const Curve25519PublicKey &ratchet_key = sender_chain[0].ratchet_key;

    axolotl::MessageWriter writer(axolotl::encode_message(
        PROTOCOL_VERSION, counter, key_length, padded, cipher_text
    ));

    std::memcpy(writer.ratchet_key, ratchet_key.public_key, key_length);

    axolotl::aes_encrypt_cbc(
        keys.cipher_key, keys.iv,
        plaintext, plaintext_length,
        writer.ciphertext
    );

    std::uint8_t mac[HMAC_SHA256_OUTPUT_LENGTH];
    axolotl::hmac_sha256(
        keys.mac_key, sizeof(keys.mac_key),
        ciphertext, writer.body_length,
        mac
    );
    std::memcpy(writer.mac, mac, MAC_LENGTH);

    return writer.body_length + MAC_LENGTH;
}


std::size_t decrypt_max_plaintext_length(
    std::size_t input_length
) {
    return input_length;
}


std::size_t axolotl::Session::decrypt(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * plaintext, std::size_t max_plaintext_length
) {
    if (max_plaintext_length < decrypt_max_plaintext_length(input_length)) {
        last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    axolotl::MessageReader reader(axolotl::decode_message(
        input, input_length, MAC_LENGTH
    ));

    if (reader.version != PROTOCOL_VERSION) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_VERSION;
        return std::size_t(-1);
    }

    if (reader.body_length == 0
            || reader.ratchet_key_length != Curve25519PublicKey::Length) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
        return std::size_t(-1);
    }

    ReceiverChain * chain = NULL;
    for (axolotl::ReceiverChain & receiver_chain : receiver_chains) {
        if (0 == std::memcmp(
                receiver_chain.ratchet_key, reader.ratchet_key, KEY_LENGTH
        )) {
            chain = &receiver_chain;
            break;
        }
    }

    if (!chain) {
        if (!verify_mac_for_new_chain(*this, input, reader)) {
            last_error = axolotl::ErrorCode::BAD_MESSAGE_MAC;
            return std::size_t(-1);
        }
    } else {
        if (chain->index > reader.counter) {
            /* Chain already advanced beyond the key for this message
             * Check if the message keys are in the skipped key list. */
            for (const axolotl::SkippedMessageKey & skipped
                    : skipped_message_keys) {
                if (reader.counter == skipped.message_key.index
                        && 0 == std::memcmp(
                            skipped.ratchet_key, reader.ratchet_key, KEY_LENGTH
                        )) {
                    /* Found the key for this message. Check the MAC. */
                    if (!verify_mac(skipped.message_key, input, reader)) {
                        last_error = axolotl::ErrorCode::BAD_MESSAGE_MAC;
                        return std::size_t(-1);
                    }

                    std::size_t result = axolotl::aes_decrypt_cbc(
                        skipped.message_key.cipher_key,
                        skipped.message_key.iv,
                        reader.ciphertext, reader.ciphertext_length,
                        plaintext
                    );

                    if (result == std::size_t(-1)) {
                        last_error = axolotl::ErrorCode::BAD_MESSAGE_MAC;
                        return result;
                    }

                    /* Remove the key from the skipped keys now that we've
                     * decoded the message it corresponds to. */
                    skipped_message_keys.erase(&skipped);
                    return result;
                }
            }
            /* No matching keys for the message, fail with bad mac */
            last_error = axolotl::ErrorCode::BAD_MESSAGE_MAC;
            return std::size_t(-1);
        } else if (!verify_mac_for_existing_chain(*chain, input, reader)) {
            last_error = axolotl::ErrorCode::BAD_MESSAGE_MAC;
            return std::size_t(-1);
        }
    }

    if (!chain) {
        
    }





}
