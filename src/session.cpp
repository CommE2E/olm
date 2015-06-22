#include "axolotl/session.hh"
#include "axolotl/cipher.hh"
#include "axolotl/crypto.hh"
#include "axolotl/account.hh"
#include "axolotl/memory.hh"
#include "axolotl/message.hh"

#include <cstring>

namespace {

static const std::size_t KEY_LENGTH = 32;
static const std::uint8_t PROTOCOL_VERSION = 0x3;

static const std::uint8_t ROOT_KDF_INFO[] = "AXOLOTL_ROOT";
static const std::uint8_t RATCHET_KDF_INFO[] = "AXOLOTL_RATCHET";
static const std::uint8_t CIPHER_KDF_INFO[] = "AXOLOTL_KEYS";

static const axolotl::CipherAesSha256 AXOLOTL_CIPHER(
    CIPHER_KDF_INFO, sizeof(CIPHER_KDF_INFO) -1
);

static const axolotl::KdfInfo AXOLOTL_KDF_INFO = {
    ROOT_KDF_INFO, sizeof(ROOT_KDF_INFO) - 1,
    RATCHET_KDF_INFO, sizeof(RATCHET_KDF_INFO) - 1
};

} // namespace

axolotl::Session::Session(
) : ratchet(AXOLOTL_KDF_INFO, AXOLOTL_CIPHER),
    last_error(axolotl::ErrorCode::SUCCESS),
    received_message(false),
    bob_one_time_key_id(0) {

}


std::size_t axolotl::Session::new_outbound_session_random_length() {
    return KEY_LENGTH * 2;
}


std::size_t axolotl::Session::new_outbound_session(
    axolotl::Account const & local_account,
    axolotl::Curve25519PublicKey const & identity_key,
    axolotl::RemoteKey const & one_time_key,
    std::uint8_t const * random, std::size_t random_length
) {
    if (random_length < new_outbound_session_random_length()) {
        last_error = axolotl::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }

    Curve25519KeyPair base_key;
    axolotl::generate_key(random, base_key);

    Curve25519KeyPair ratchet_key;
    axolotl::generate_key(random + 32, ratchet_key);

    received_message = false;
    alice_identity_key.id = local_account.identity_key.id;
    alice_identity_key.key = local_account.identity_key.key;
    alice_base_key = base_key;
    bob_one_time_key_id = one_time_key.id;

    std::uint8_t shared_secret[96];

    axolotl::curve25519_shared_secret(
        local_account.identity_key.key, one_time_key.key, shared_secret
    );
    axolotl::curve25519_shared_secret(
         base_key, identity_key, shared_secret + 32
    );
    axolotl::curve25519_shared_secret(
         base_key, one_time_key.key, shared_secret + 64
    );

    ratchet.initialise_as_alice(shared_secret, 96, ratchet_key);

    axolotl::unset(base_key);
    axolotl::unset(ratchet_key);
    axolotl::unset(shared_secret);

    return std::size_t(0);
}

namespace {

bool check_message_fields(
    axolotl::PreKeyMessageReader & reader
) {
    bool ok = true;
    ok = ok && reader.identity_key;
    ok = ok && reader.identity_key_length == KEY_LENGTH;
    ok = ok && reader.message;
    ok = ok && reader.base_key;
    ok = ok && reader.base_key_length == KEY_LENGTH;
    ok = ok && reader.has_one_time_key_id;
    ok = ok && reader.has_registration_id;
    return ok;
}

} // namespace


std::size_t axolotl::Session::new_inbound_session(
    axolotl::Account & local_account,
    std::uint8_t const * one_time_key_message, std::size_t message_length
) {
    axolotl::PreKeyMessageReader reader;
    decode_one_time_key_message(reader, one_time_key_message, message_length);

    if (!check_message_fields(reader)) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
        return std::size_t(-1);
    }

    axolotl::MessageReader message_reader;
    decode_message(
        message_reader, reader.message, reader.message_length,
        ratchet.ratchet_cipher.mac_length()
    );

    if (!message_reader.ratchet_key
            || message_reader.ratchet_key_length != KEY_LENGTH) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
        return std::size_t(-1);
    }

    alice_identity_key.id = reader.registration_id;
    std::memcpy(alice_identity_key.key.public_key, reader.identity_key, 32);
    std::memcpy(alice_base_key.public_key, reader.base_key, 32);
    bob_one_time_key_id = reader.one_time_key_id;
    axolotl::Curve25519PublicKey ratchet_key;
    std::memcpy(ratchet_key.public_key, message_reader.ratchet_key, 32);

    axolotl::LocalKey const * bob_one_time_key = local_account.lookup_key(
        bob_one_time_key_id
    );

    if (!bob_one_time_key) {
        last_error = axolotl::ErrorCode::BAD_MESSAGE_KEY_ID;
        return std::size_t(-1);
    }

    std::uint8_t shared_secret[96];

    axolotl::curve25519_shared_secret(
        bob_one_time_key->key, alice_identity_key.key, shared_secret
    );
    axolotl::curve25519_shared_secret(
        local_account.identity_key.key, alice_base_key, shared_secret + 32
    );
    axolotl::curve25519_shared_secret(
        bob_one_time_key->key, alice_base_key, shared_secret + 64
    );

    ratchet.initialise_as_bob(shared_secret, 96, ratchet_key);

    return std::size_t(0);
}


bool axolotl::Session::matches_inbound_session(
    std::uint8_t const * one_time_key_message, std::size_t message_length
) {
    axolotl::PreKeyMessageReader reader;
    decode_one_time_key_message(reader, one_time_key_message, message_length);

    if (!check_message_fields(reader)) {
        return false;
    }

    bool same = true;
    same = same && 0 == std::memcmp(
        reader.identity_key, alice_identity_key.key.public_key, KEY_LENGTH
    );
    same = same && 0 == std::memcmp(
        reader.base_key, alice_base_key.public_key, KEY_LENGTH
    );
    same = same && reader.one_time_key_id == bob_one_time_key_id;
    same = same && reader.registration_id == alice_identity_key.id;
    return same;
}


axolotl::MessageType axolotl::Session::encrypt_message_type() {
    if (received_message) {
        return axolotl::MessageType::MESSAGE;
    } else {
        return axolotl::MessageType::PRE_KEY_MESSAGE;
    }
}


std::size_t axolotl::Session::encrypt_message_length(
    std::size_t plaintext_length
) {
    std::size_t message_length = ratchet.encrypt_output_length(
        plaintext_length
    );

    if (received_message) {
        return message_length;
    }

    return encode_one_time_key_message_length(
        alice_identity_key.id,
        bob_one_time_key_id,
        KEY_LENGTH,
        KEY_LENGTH,
        message_length
    );
}


std::size_t axolotl::Session::encrypt_random_length() {
    return ratchet.encrypt_random_length();
}


std::size_t axolotl::Session::encrypt(
    std::uint8_t const * plaintext, std::size_t plaintext_length,
    std::uint8_t const * random, std::size_t random_length,
    std::uint8_t * message, std::size_t message_length
) {
    if (message_length < encrypt_message_length(plaintext_length)) {
        last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    std::uint8_t * message_body;
    std::size_t message_body_length = ratchet.encrypt_output_length(
        plaintext_length
    );

    if (received_message) {
        message_body = message;
    } else {
        axolotl::PreKeyMessageWriter writer;
        encode_one_time_key_message(
            writer,
            PROTOCOL_VERSION,
            alice_identity_key.id,
            bob_one_time_key_id,
            KEY_LENGTH,
            KEY_LENGTH,
            message_body_length,
            message
        );
        std::memcpy(
            writer.identity_key, alice_identity_key.key.public_key, KEY_LENGTH
        );
        std::memcpy(
            writer.base_key, alice_base_key.public_key, KEY_LENGTH
        );
        message_body = writer.message;
    }

    std::size_t result = ratchet.encrypt(
        plaintext, plaintext_length,
        random, random_length,
        message_body, message_body_length
    );

    if (result == std::size_t(-1)) {
        last_error = ratchet.last_error;
        ratchet.last_error = axolotl::ErrorCode::SUCCESS;
    }
    return result;
}


std::size_t axolotl::Session::decrypt_max_plaintext_length(
    MessageType message_type,
    std::uint8_t const * message, std::size_t message_length
) {
    std::uint8_t const * message_body;
    std::size_t message_body_length;
    if (message_type == axolotl::MessageType::MESSAGE) {
        message_body = message;
        message_body_length = message_length;
    } else {
        axolotl::PreKeyMessageReader reader;
        decode_one_time_key_message(reader, message, message_length);
        if (!reader.message) {
            last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
            return std::size_t(-1);
        }
        message_body = reader.message;
        message_body_length = reader.message_length;
    }

    std::size_t result = ratchet.decrypt_max_plaintext_length(
        message_body, message_body_length
    );

    if (result == std::size_t(-1)) {
        last_error = ratchet.last_error;
        ratchet.last_error = axolotl::ErrorCode::SUCCESS;
    }
    return result;
}


std::size_t axolotl::Session::decrypt(
    axolotl::MessageType message_type,
    std::uint8_t const * message, std::size_t message_length,
    std::uint8_t * plaintext, std::size_t max_plaintext_length
) {
    std::uint8_t const * message_body;
    std::size_t message_body_length;
    if (message_type == axolotl::MessageType::MESSAGE) {
        message_body = message;
        message_body_length = message_length;
    } else {
        axolotl::PreKeyMessageReader reader;
        decode_one_time_key_message(reader, message, message_length);
        if (!reader.message) {
            last_error = axolotl::ErrorCode::BAD_MESSAGE_FORMAT;
            return std::size_t(-1);
        }
        message_body = reader.message;
        message_body_length = reader.message_length;
    }

    std::size_t result = ratchet.decrypt(
        message_body, message_body_length, plaintext, max_plaintext_length
    );

    if (result == std::size_t(-1)) {
        last_error = ratchet.last_error;
        ratchet.last_error = axolotl::ErrorCode::SUCCESS;
    }
    return result;
}
