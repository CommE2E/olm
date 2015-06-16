#include "axolotl/axolotl.hh"
#include "axolotl/session.hh"
#include "axolotl/account.hh"
#include "axolotl/base64.hh"
#include "axolotl/cipher.hh"

#include <new>
#include <cstring>

namespace {

static AxolotlAccount * to_c(axolotl::Account * account) {
    return reinterpret_cast<AxolotlAccount *>(account);
}

static AxolotlSession * to_c(axolotl::Session * account) {
    return reinterpret_cast<AxolotlSession *>(account);
}

static axolotl::Account * from_c(AxolotlAccount * account) {
    return reinterpret_cast<axolotl::Account *>(account);
}

static axolotl::Session * from_c(AxolotlSession * account) {
    return reinterpret_cast<axolotl::Session *>(account);
}

static std::uint8_t * from_c(void * bytes) {
    return reinterpret_cast<std::uint8_t *>(bytes);
}

static std::uint8_t const * from_c(void const * bytes) {
    return reinterpret_cast<std::uint8_t const *>(bytes);
}

static const std::uint8_t CIPHER_KDF_INFO[] = "Pickle";

static const axolotl::CipherAesSha256 PICKLE_CIPHER(
    CIPHER_KDF_INFO, sizeof(CIPHER_KDF_INFO) -1
);

std::size_t enc_output_length(
    size_t raw_length
) {
    std::size_t length = PICKLE_CIPHER.encrypt_ciphertext_length(raw_length);
    length += PICKLE_CIPHER.mac_length();
    return axolotl::encode_base64_length(length);
}


std::uint8_t * enc_output_pos(
    std::uint8_t * output,
    size_t raw_length
) {
    std::size_t length = PICKLE_CIPHER.encrypt_ciphertext_length(raw_length);
    length += PICKLE_CIPHER.mac_length();
    return output + axolotl::encode_base64_length(length) - length;
}

std::size_t enc_output(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t * output, size_t raw_length
) {
    std::size_t ciphertext_length = PICKLE_CIPHER.encrypt_ciphertext_length(
        raw_length
    );
    std::size_t length = ciphertext_length + PICKLE_CIPHER.mac_length();
    std::size_t base64_length = axolotl::encode_base64_length(length);
    std::uint8_t * raw_output = output + base64_length - length;
    PICKLE_CIPHER.encrypt(
        key, key_length,
        raw_output, raw_length,
        raw_output, ciphertext_length,
        raw_output, length
    );
    axolotl::encode_base64(raw_output, length, output);
    return raw_length;
}

std::size_t enc_input(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t * input, size_t b64_length,
    axolotl::ErrorCode & last_error
) {
    std::size_t enc_length = axolotl::decode_base64_length(b64_length);
    if (enc_length == std::size_t(-1)) {
        last_error = axolotl::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    axolotl::decode_base64(input, b64_length, input);
    std::size_t raw_length = enc_length - PICKLE_CIPHER.mac_length();
    std::size_t result = PICKLE_CIPHER.decrypt(
        key, key_length,
        input, enc_length,
        input, raw_length,
        input, raw_length
    );
    if (result == std::size_t(-1)) {
        last_error = axolotl::ErrorCode::BAD_ACCOUNT_KEY;
    }
    return result;
}


std::size_t b64_output_length(
    size_t raw_length
) {
    return axolotl::encode_base64_length(raw_length);
}

std::uint8_t * b64_output_pos(
    std::uint8_t * output,
    size_t raw_length
) {
    return output + axolotl::encode_base64_length(raw_length) - raw_length;
}

std::size_t b64_output(
    std::uint8_t * output, size_t raw_length
) {
    std::size_t base64_length = axolotl::encode_base64_length(raw_length);
    std::uint8_t * raw_output = output + base64_length - raw_length;
    axolotl::encode_base64(raw_output, raw_length, output);
    return base64_length;
}

std::size_t b64_input(
    std::uint8_t * input, size_t b64_length,
    axolotl::ErrorCode & last_error
) {
    std::size_t raw_length = axolotl::decode_base64_length(b64_length);
    if (raw_length == std::size_t(-1)) {
        last_error = axolotl::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    axolotl::decode_base64(input, b64_length, input);
    return raw_length;
}

const char * errors[9] {
    "SUCCESS",
    "NOT_ENOUGH_RANDOM",
    "OUTPUT_BUFFER_TOO_SMALL",
    "BAD_MESSAGE_VERSION",
    "BAD_MESSAGE_FORMAT",
    "BAD_MESSAGE_MAC",
    "BAD_MESSAGE_KEY_ID",
    "INVALID_BASE64",
    "BAD_ACCOUNT_KEY",
};

} // namespace


extern "C" {


size_t axolotl_error() {
    return std::size_t(-1);
}


const char * axolotl_account_last_error(
    AxolotlSession * account
) {
    unsigned error = unsigned(from_c(account)->last_error);
    if (error < 9) {
        return errors[error];
    } else {
        return "UNKNOWN_ERROR";
    }
}


const char * axolotl_session_last_error(
    AxolotlSession * session
) {
    unsigned error = unsigned(from_c(session)->last_error);
    if (error < 9) {
        return errors[error];
    } else {
        return "UNKNOWN_ERROR";
    }
}


size_t axolotl_account_size() {
    return sizeof(axolotl::Account);
}


size_t axolotl_session_size() {
    return sizeof(axolotl::Session);
}


AxolotlAccount * axolotl_account(
    void * memory
) {
    return to_c(new(memory) axolotl::Account());
}


AxolotlSession * axolotl_session(
    void * memory
) {
    return to_c(new(memory) axolotl::Session());
}


size_t axolotl_pickle_account_length(
    AxolotlAccount * account
) {
    return enc_output_length(pickle_length(*from_c(account)));
}


size_t axolotl_pickle_session_length(
    AxolotlSession * session
) {
    return enc_output_length(pickle_length(*from_c(session)));
}


size_t axolotl_pickle_account(
    AxolotlAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    axolotl::Account & object = *from_c(account);
    std::size_t raw_length = pickle_length(object);
    if (pickled_length < enc_output_length(raw_length)) {
        object.last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return size_t(-1);
    }
    pickle(enc_output_pos(from_c(pickled), raw_length), object);
    return enc_output(from_c(key), key_length, from_c(pickled), raw_length);
}


size_t axolotl_pickle_session(
    AxolotlSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    axolotl::Session & object = *from_c(session);
    std::size_t raw_length = pickle_length(object);
    if (pickled_length < enc_output_length(raw_length)) {
        object.last_error = axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return size_t(-1);
    }
    pickle(enc_output_pos(from_c(pickled), raw_length), object);
    return enc_output(from_c(key), key_length, from_c(pickled), raw_length);
}


size_t axolotl_unpickle_account(
    AxolotlAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    axolotl::Account & object = *from_c(account);
    std::uint8_t * const pos = from_c(pickled);
    std::size_t raw_length = enc_input(
        from_c(key), key_length, pos, pickled_length, object.last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    std::uint8_t * const end = pos + raw_length;
    unpickle(pos, end, object);
    return pickled_length;
}


size_t axolotl_unpickle_session(
    AxolotlSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    axolotl::Session & object = *from_c(session);
    std::uint8_t * const pos = from_c(pickled);
    std::size_t raw_length = enc_input(
        from_c(key), key_length, pos, pickled_length, object.last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    std::uint8_t * const end = pos + raw_length;
    unpickle(pos, end, object);
    return pickled_length;
}


size_t axolotl_create_account_random_length(
    AxolotlAccount * account
) {
    return from_c(account)->new_account_random_length();
}


size_t axolotl_create_account(
    AxolotlAccount * account,
    void const * random, size_t random_length
) {
    return from_c(account)->new_account(from_c(random), random_length);
}

namespace {

static const std::size_t OUTPUT_KEY_LENGTH = 2 + 10 + 2 +
        axolotl::encode_base64_length(32) + 3;

void output_key(
    axolotl::LocalKey const & key,
    std::uint8_t sep,
    std::uint8_t * output
) {
    output[0] = sep;
    output[1] = '[';
    std::memset(output + 2, ' ', 10);
    uint32_t value = key.id;
    uint8_t * number = output + 11;
    *number = '0' + value % 10;
    value /= 10;
    while (value) {
        *(--number) = '0' + value % 10;
        value /= 10;
    }
    output[12] = ',';
    output[13] = '"';
    axolotl::encode_base64(key.key.public_key, 32, output + 14);
    output[OUTPUT_KEY_LENGTH - 3] = '"';
    output[OUTPUT_KEY_LENGTH - 2] = ']';
    output[OUTPUT_KEY_LENGTH - 1] = '\n';
}

} // namespace


size_t axolotl_account_identity_keys_length(
    AxolotlAccount * account
) {
    return OUTPUT_KEY_LENGTH * 2 + 1;
}


size_t axolotl_account_identity_keys(
    AxolotlAccount * account,
    void * identity_keys, size_t identity_key_length
) {
    std::size_t length = OUTPUT_KEY_LENGTH * 2 + 1;
    if (identity_key_length < length) {
        from_c(account)->last_error =
            axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return size_t(-1);
    }
    std::uint8_t * output = from_c(identity_keys);
    output_key(from_c(account)->identity_key, '[', output);
    output += OUTPUT_KEY_LENGTH;
    output += OUTPUT_KEY_LENGTH;
    output[0] = ']';
    return length;
}


size_t axolotl_account_one_time_keys_length(
    AxolotlAccount * account
) {
    size_t count = from_c(account)->one_time_keys.size();
    return OUTPUT_KEY_LENGTH * (count + 1) + 1;
}


size_t axolotl_account_one_time_keys(
    AxolotlAccount * account,
    void * identity_keys, size_t identity_key_length
) {
    std::size_t length = axolotl_account_one_time_keys_length(account);
    if (identity_key_length < length) {
        from_c(account)->last_error =
            axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return size_t(-1);
    }
    std::uint8_t * output = from_c(identity_keys);
    output_key(from_c(account)->last_resort_one_time_key, '[', output);
    output += OUTPUT_KEY_LENGTH;
    for (auto const & key : from_c(account)->one_time_keys) {
        output_key(key, ',', output);
        output += OUTPUT_KEY_LENGTH;
    }
    output[0] = ']';
    return length;
}


size_t axolotl_create_outbound_session_random_length(
    AxolotlSession * session
) {
    return from_c(session)->new_outbound_session_random_length();
}

size_t axolotl_create_outbound_session(
    AxolotlSession * session,
    AxolotlAccount * account,
    void const * their_identity_key, size_t their_identity_key_length,
    unsigned their_one_time_key_id,
    void const * their_one_time_key, size_t their_one_time_key_length,
    void const * random, size_t random_length
) {
    if (axolotl::decode_base64_length(their_identity_key_length) != 32
            || axolotl::decode_base64_length(their_one_time_key_length) != 32
    ) {
        from_c(session)->last_error = axolotl::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    axolotl::Curve25519PublicKey identity_key;
    axolotl::RemoteKey one_time_key;

    axolotl::decode_base64(
        from_c(their_identity_key), their_identity_key_length,
        identity_key.public_key
    );
    one_time_key.id = their_one_time_key_id;
    axolotl::decode_base64(
        from_c(their_one_time_key), their_one_time_key_length,
        one_time_key.key.public_key
    );

    return from_c(session)->new_outbound_session(
        *from_c(account), identity_key, one_time_key,
        from_c(random), random_length
    );
}


size_t axolotl_create_inbound_session(
    AxolotlSession * session,
    AxolotlAccount * account,
    void * one_time_key_message, size_t message_length
) {
    std::size_t raw_length = b64_input(
        from_c(one_time_key_message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    return from_c(session)->new_inbound_session(
        *from_c(account), from_c(one_time_key_message), raw_length
    );
}


size_t axolotl_matches_inbound_session(
    AxolotlSession * session,
    void * one_time_key_message, size_t message_length
) {
    std::size_t raw_length = b64_input(
        from_c(one_time_key_message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    bool matches = from_c(session)->matches_inbound_session(
        from_c(one_time_key_message), raw_length
    );
    return matches ? 1 : 0;
}


size_t axolotl_encrypt_message_type(
    AxolotlSession * session
) {
    return size_t(from_c(session)->encrypt_message_type());
}


size_t axolotl_encrypt_random_length(
    AxolotlSession * session
) {
    return from_c(session)->encrypt_random_length();
}


size_t axolotl_encrypt_message_length(
    AxolotlSession * session,
    size_t plaintext_length
) {
    return b64_output_length(
        from_c(session)->encrypt_message_length(plaintext_length)
    );
}


size_t axolotl_encrypt(
    AxolotlSession * session,
    void const * plaintext, size_t plaintext_length,
    void const * random, size_t random_length,
    void * message, size_t message_length
) {
    std::size_t raw_length = from_c(session)->encrypt_message_length(
        plaintext_length
    );
    if (message_length < raw_length) {
        from_c(session)->last_error =
            axolotl::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    from_c(session)->encrypt(
        from_c(plaintext), plaintext_length,
        from_c(random), random_length,
        b64_output_pos(from_c(message), raw_length), raw_length
    );
    return b64_output(from_c(message), raw_length);
}


size_t axolotl_decrypt_max_plaintext_length(
    AxolotlSession * session,
    size_t message_type,
    void * message, size_t message_length
) {
    std::size_t raw_length = b64_input(
        from_c(message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    return from_c(session)->decrypt_max_plaintext_length(
        axolotl::MessageType(message_type), from_c(message), raw_length
    );
}


size_t axolotl_decrypt(
    AxolotlSession * session,
    size_t message_type,
    void * message, size_t message_length,
    void * plaintext, size_t max_plaintext_length
) {
    std::size_t raw_length = b64_input(
        from_c(message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    return from_c(session)->decrypt(
        axolotl::MessageType(message_type), from_c(message), raw_length,
        from_c(plaintext), max_plaintext_length
    );
}

}
