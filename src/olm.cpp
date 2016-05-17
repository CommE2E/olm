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
#include "olm/olm.h"
#include "olm/session.hh"
#include "olm/account.hh"
#include "olm/utility.hh"
#include "olm/base64.hh"
#include "olm/cipher.hh"
#include "olm/memory.hh"

#include <new>
#include <cstring>

namespace {

static OlmAccount * to_c(olm::Account * account) {
    return reinterpret_cast<OlmAccount *>(account);
}

static OlmSession * to_c(olm::Session * session) {
    return reinterpret_cast<OlmSession *>(session);
}

static OlmUtility * to_c(olm::Utility * utility) {
    return reinterpret_cast<OlmUtility *>(utility);
}

static olm::Account * from_c(OlmAccount * account) {
    return reinterpret_cast<olm::Account *>(account);
}

static olm::Session * from_c(OlmSession * session) {
    return reinterpret_cast<olm::Session *>(session);
}

static olm::Utility * from_c(OlmUtility * utility) {
    return reinterpret_cast<olm::Utility *>(utility);
}

static std::uint8_t * from_c(void * bytes) {
    return reinterpret_cast<std::uint8_t *>(bytes);
}

static std::uint8_t const * from_c(void const * bytes) {
    return reinterpret_cast<std::uint8_t const *>(bytes);
}

static const std::uint8_t CIPHER_KDF_INFO[] = "Pickle";

static const olm::CipherAesSha256 PICKLE_CIPHER(
    CIPHER_KDF_INFO, sizeof(CIPHER_KDF_INFO) -1
);

std::size_t enc_output_length(
    size_t raw_length
) {
    std::size_t length = PICKLE_CIPHER.encrypt_ciphertext_length(raw_length);
    length += PICKLE_CIPHER.mac_length();
    return olm::encode_base64_length(length);
}


std::uint8_t * enc_output_pos(
    std::uint8_t * output,
    size_t raw_length
) {
    std::size_t length = PICKLE_CIPHER.encrypt_ciphertext_length(raw_length);
    length += PICKLE_CIPHER.mac_length();
    return output + olm::encode_base64_length(length) - length;
}

std::size_t enc_output(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t * output, size_t raw_length
) {
    std::size_t ciphertext_length = PICKLE_CIPHER.encrypt_ciphertext_length(
        raw_length
    );
    std::size_t length = ciphertext_length + PICKLE_CIPHER.mac_length();
    std::size_t base64_length = olm::encode_base64_length(length);
    std::uint8_t * raw_output = output + base64_length - length;
    PICKLE_CIPHER.encrypt(
        key, key_length,
        raw_output, raw_length,
        raw_output, ciphertext_length,
        raw_output, length
    );
    olm::encode_base64(raw_output, length, output);
    return raw_length;
}

std::size_t enc_input(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t * input, size_t b64_length,
    olm::ErrorCode & last_error
) {
    std::size_t enc_length = olm::decode_base64_length(b64_length);
    if (enc_length == std::size_t(-1)) {
        last_error = olm::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    olm::decode_base64(input, b64_length, input);
    std::size_t raw_length = enc_length - PICKLE_CIPHER.mac_length();
    std::size_t result = PICKLE_CIPHER.decrypt(
        key, key_length,
        input, enc_length,
        input, raw_length,
        input, raw_length
    );
    if (result == std::size_t(-1)) {
        last_error = olm::ErrorCode::BAD_ACCOUNT_KEY;
    }
    return result;
}


std::size_t b64_output_length(
    size_t raw_length
) {
    return olm::encode_base64_length(raw_length);
}

std::uint8_t * b64_output_pos(
    std::uint8_t * output,
    size_t raw_length
) {
    return output + olm::encode_base64_length(raw_length) - raw_length;
}

std::size_t b64_output(
    std::uint8_t * output, size_t raw_length
) {
    std::size_t base64_length = olm::encode_base64_length(raw_length);
    std::uint8_t * raw_output = output + base64_length - raw_length;
    olm::encode_base64(raw_output, raw_length, output);
    return base64_length;
}

std::size_t b64_input(
    std::uint8_t * input, size_t b64_length,
    olm::ErrorCode & last_error
) {
    std::size_t raw_length = olm::decode_base64_length(b64_length);
    if (raw_length == std::size_t(-1)) {
        last_error = olm::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    olm::decode_base64(input, b64_length, input);
    return raw_length;
}

static const char * ERRORS[11] {
    "SUCCESS",
    "NOT_ENOUGH_RANDOM",
    "OUTPUT_BUFFER_TOO_SMALL",
    "BAD_MESSAGE_VERSION",
    "BAD_MESSAGE_FORMAT",
    "BAD_MESSAGE_MAC",
    "BAD_MESSAGE_KEY_ID",
    "INVALID_BASE64",
    "BAD_ACCOUNT_KEY",
    "UNKNOWN_PICKLE_VERSION",
    "CORRUPTED_PICKLE",
};

} // namespace


extern "C" {


size_t olm_error() {
    return std::size_t(-1);
}


const char * olm_account_last_error(
    OlmAccount * account
) {
    unsigned error = unsigned(from_c(account)->last_error);
    if (error < (sizeof(ERRORS)/sizeof(ERRORS[0]))) {
        return ERRORS[error];
    } else {
        return "UNKNOWN_ERROR";
    }
}


const char * olm_session_last_error(
    OlmSession * session
) {
    unsigned error = unsigned(from_c(session)->last_error);
    if (error < (sizeof(ERRORS)/sizeof(ERRORS[0]))) {
        return ERRORS[error];
    } else {
        return "UNKNOWN_ERROR";
    }
}

const char * olm_utility_last_error(
    OlmUtility * utility
) {
    unsigned error = unsigned(from_c(utility)->last_error);
    if (error < (sizeof(ERRORS)/sizeof(ERRORS[0]))) {
        return ERRORS[error];
    } else {
        return "UNKNOWN_ERROR";
    }
}

size_t olm_account_size() {
    return sizeof(olm::Account);
}


size_t olm_session_size() {
    return sizeof(olm::Session);
}

size_t olm_utility_size() {
    return sizeof(olm::Utility);
}

OlmAccount * olm_account(
    void * memory
) {
    olm::unset(memory, sizeof(olm::Account));
    return to_c(new(memory) olm::Account());
}


OlmSession * olm_session(
    void * memory
) {
    olm::unset(memory, sizeof(olm::Session));
    return to_c(new(memory) olm::Session());
}


OlmUtility * olm_utility(
    void * memory
) {
    olm::unset(memory, sizeof(olm::Utility));
    return to_c(new(memory) olm::Utility());
}


size_t olm_clear_account(
    OlmAccount * account
) {
    /* Clear the memory backing the account  */
    olm::unset(account, sizeof(olm::Account));
    /* Initialise a fresh account object in case someone tries to use it */
    new(account) olm::Account();
    return sizeof(olm::Account);
}


size_t olm_clear_session(
    OlmSession * session
) {
    /* Clear the memory backing the session */
    olm::unset(session, sizeof(olm::Session));
    /* Initialise a fresh session object in case someone tries to use it */
    new(session) olm::Session();
    return sizeof(olm::Session);
}


size_t olm_clear_utility(
    OlmUtility * utility
) {
    /* Clear the memory backing the session */
    olm::unset(utility, sizeof(olm::Utility));
    /* Initialise a fresh session object in case someone tries to use it */
    new(utility) olm::Utility();
    return sizeof(olm::Utility);
}


size_t olm_pickle_account_length(
    OlmAccount * account
) {
    return enc_output_length(pickle_length(*from_c(account)));
}


size_t olm_pickle_session_length(
    OlmSession * session
) {
    return enc_output_length(pickle_length(*from_c(session)));
}


size_t olm_pickle_account(
    OlmAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    olm::Account & object = *from_c(account);
    std::size_t raw_length = pickle_length(object);
    if (pickled_length < enc_output_length(raw_length)) {
        object.last_error = olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return size_t(-1);
    }
    pickle(enc_output_pos(from_c(pickled), raw_length), object);
    return enc_output(from_c(key), key_length, from_c(pickled), raw_length);
}


size_t olm_pickle_session(
    OlmSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    olm::Session & object = *from_c(session);
    std::size_t raw_length = pickle_length(object);
    if (pickled_length < enc_output_length(raw_length)) {
        object.last_error = olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return size_t(-1);
    }
    pickle(enc_output_pos(from_c(pickled), raw_length), object);
    return enc_output(from_c(key), key_length, from_c(pickled), raw_length);
}


size_t olm_unpickle_account(
    OlmAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    olm::Account & object = *from_c(account);
    std::uint8_t * const pos = from_c(pickled);
    std::size_t raw_length = enc_input(
        from_c(key), key_length, pos, pickled_length, object.last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    std::uint8_t * const end = pos + raw_length;
    /* On success unpickle will return (pos + raw_length). If unpickling
     * terminates too soon then it will return a pointer before
     * (pos + raw_length). On error unpickle will return (pos + raw_length + 1).
     */
    if (end != unpickle(pos, end + 1, object)) {
        if (object.last_error == olm::ErrorCode::SUCCESS) {
            object.last_error = olm::ErrorCode::CORRUPTED_PICKLE;
        }
        return std::size_t(-1);
    }
    return pickled_length;
}


size_t olm_unpickle_session(
    OlmSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
) {
    olm::Session & object = *from_c(session);
    std::uint8_t * const pos = from_c(pickled);
    std::size_t raw_length = enc_input(
        from_c(key), key_length, pos, pickled_length, object.last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }

    std::uint8_t * const end = pos + raw_length;
    /* On success unpickle will return (pos + raw_length). If unpickling
     * terminates too soon then it will return a pointer before
     * (pos + raw_length). On error unpickle will return (pos + raw_length + 1).
     */
    if (end != unpickle(pos, end + 1, object)) {
        if (object.last_error == olm::ErrorCode::SUCCESS) {
            object.last_error = olm::ErrorCode::CORRUPTED_PICKLE;
        }
        return std::size_t(-1);
    }
    return pickled_length;
}


size_t olm_create_account_random_length(
    OlmAccount * account
) {
    return from_c(account)->new_account_random_length();
}


size_t olm_create_account(
    OlmAccount * account,
    void * random, size_t random_length
) {
    size_t result = from_c(account)->new_account(from_c(random), random_length);
    olm::unset(random, random_length);
    return result;
}


size_t olm_account_identity_keys_length(
    OlmAccount * account
) {
    return from_c(account)->get_identity_json_length();
}


size_t olm_account_identity_keys(
    OlmAccount * account,
    void * identity_keys, size_t identity_key_length
) {
    return from_c(account)->get_identity_json(
        from_c(identity_keys), identity_key_length
    );
}


size_t olm_account_signature_length(
    OlmAccount * account
) {
    return b64_output_length(from_c(account)->signature_length());
}


size_t olm_account_sign(
    OlmAccount * account,
    void const * message, size_t message_length,
    void * signature, size_t signature_length
) {
    std::size_t raw_length = from_c(account)->signature_length();
    if (signature_length < b64_output_length(raw_length)) {
        from_c(account)->last_error =
            olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    from_c(account)->sign(
         from_c(message), message_length,
         b64_output_pos(from_c(signature), raw_length), raw_length
    );
    return b64_output(from_c(signature), raw_length);
}


size_t olm_account_one_time_keys_length(
    OlmAccount * account
) {
    return from_c(account)->get_one_time_keys_json_length();
}


size_t olm_account_one_time_keys(
    OlmAccount * account,
    void * one_time_keys_json, size_t one_time_key_json_length
) {
    return from_c(account)->get_one_time_keys_json(
        from_c(one_time_keys_json), one_time_key_json_length
    );
}


size_t olm_account_mark_keys_as_published(
    OlmAccount * account
) {
    return from_c(account)->mark_keys_as_published();
}


size_t olm_account_max_number_of_one_time_keys(
    OlmAccount * account
) {
    return from_c(account)->max_number_of_one_time_keys();
}


size_t olm_account_generate_one_time_keys_random_length(
    OlmAccount * account,
    size_t number_of_keys
) {
    return from_c(account)->generate_one_time_keys_random_length(number_of_keys);
}


size_t olm_account_generate_one_time_keys(
    OlmAccount * account,
    size_t number_of_keys,
    void * random, size_t random_length
) {
    size_t result = from_c(account)->generate_one_time_keys(
        number_of_keys,
        from_c(random), random_length
    );
    olm::unset(random, random_length);
    return result;
}


size_t olm_create_outbound_session_random_length(
    OlmSession * session
) {
    return from_c(session)->new_outbound_session_random_length();
}


size_t olm_create_outbound_session(
    OlmSession * session,
    OlmAccount * account,
    void const * their_identity_key, size_t their_identity_key_length,
    void const * their_one_time_key, size_t their_one_time_key_length,
    void * random, size_t random_length
) {
    std::uint8_t const * id_key = from_c(their_identity_key);
    std::uint8_t const * ot_key = from_c(their_one_time_key);
    std::size_t id_key_length = their_identity_key_length;
    std::size_t ot_key_length = their_one_time_key_length;

    if (olm::decode_base64_length(id_key_length) != olm::KEY_LENGTH
            || olm::decode_base64_length(ot_key_length) != olm::KEY_LENGTH
    ) {
        from_c(session)->last_error = olm::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    olm::Curve25519PublicKey identity_key;
    olm::Curve25519PublicKey one_time_key;

    olm::decode_base64(id_key, id_key_length, identity_key.public_key);
    olm::decode_base64(ot_key, ot_key_length, one_time_key.public_key);

    size_t result = from_c(session)->new_outbound_session(
        *from_c(account), identity_key, one_time_key,
        from_c(random), random_length
    );
    olm::unset(random, random_length);
    return result;
}


size_t olm_create_inbound_session(
    OlmSession * session,
    OlmAccount * account,
    void * one_time_key_message, size_t message_length
) {
    std::size_t raw_length = b64_input(
        from_c(one_time_key_message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    return from_c(session)->new_inbound_session(
        *from_c(account), nullptr, from_c(one_time_key_message), raw_length
    );
}


size_t olm_create_inbound_session_from(
    OlmSession * session,
    OlmAccount * account,
    void const * their_identity_key, size_t their_identity_key_length,
    void * one_time_key_message, size_t message_length
) {
    std::uint8_t const * id_key = from_c(their_identity_key);
    std::size_t id_key_length = their_identity_key_length;

    if (olm::decode_base64_length(id_key_length) != olm::KEY_LENGTH) {
        from_c(session)->last_error = olm::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    olm::Curve25519PublicKey identity_key;
    olm::decode_base64(id_key, id_key_length, identity_key.public_key);

    std::size_t raw_length = b64_input(
        from_c(one_time_key_message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    return from_c(session)->new_inbound_session(
        *from_c(account), &identity_key,
        from_c(one_time_key_message), raw_length
    );
}


size_t olm_session_id_length(
    OlmSession * session
) {
    return b64_output_length(from_c(session)->session_id_length());
}

size_t olm_session_id(
    OlmSession * session,
    void * id, size_t id_length
) {
    std::size_t raw_length = from_c(session)->session_id_length();
    if (id_length < b64_output_length(raw_length)) {
        from_c(session)->last_error =
                olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    std::size_t result = from_c(session)->session_id(
       b64_output_pos(from_c(id), raw_length), raw_length
    );
    if (result == std::size_t(-1)) {
        return result;
    }
    return b64_output(from_c(id), raw_length);
}


size_t olm_matches_inbound_session(
    OlmSession * session,
    void * one_time_key_message, size_t message_length
) {
    std::size_t raw_length = b64_input(
        from_c(one_time_key_message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    bool matches = from_c(session)->matches_inbound_session(
        nullptr, from_c(one_time_key_message), raw_length
    );
    return matches ? 1 : 0;
}


size_t olm_matches_inbound_session_from(
    OlmSession * session,
    void const * their_identity_key, size_t their_identity_key_length,
    void * one_time_key_message, size_t message_length
) {
    std::uint8_t const * id_key = from_c(their_identity_key);
    std::size_t id_key_length = their_identity_key_length;

    if (olm::decode_base64_length(id_key_length) != olm::KEY_LENGTH) {
        from_c(session)->last_error = olm::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    olm::Curve25519PublicKey identity_key;
    olm::decode_base64(id_key, id_key_length, identity_key.public_key);

    std::size_t raw_length = b64_input(
        from_c(one_time_key_message), message_length, from_c(session)->last_error
    );
    if (raw_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    bool matches = from_c(session)->matches_inbound_session(
        &identity_key, from_c(one_time_key_message), raw_length
    );
    return matches ? 1 : 0;
}


size_t olm_remove_one_time_keys(
    OlmAccount * account,
    OlmSession * session
) {
    size_t result = from_c(account)->remove_key(
        from_c(session)->bob_one_time_key
    );
    if (result == std::size_t(-1)) {
        from_c(account)->last_error = olm::ErrorCode::BAD_MESSAGE_KEY_ID;
    }
    return result;
}


size_t olm_encrypt_message_type(
    OlmSession * session
) {
    return size_t(from_c(session)->encrypt_message_type());
}


size_t olm_encrypt_random_length(
    OlmSession * session
) {
    return from_c(session)->encrypt_random_length();
}


size_t olm_encrypt_message_length(
    OlmSession * session,
    size_t plaintext_length
) {
    return b64_output_length(
        from_c(session)->encrypt_message_length(plaintext_length)
    );
}


size_t olm_encrypt(
    OlmSession * session,
    void const * plaintext, size_t plaintext_length,
    void * random, size_t random_length,
    void * message, size_t message_length
) {
    std::size_t raw_length = from_c(session)->encrypt_message_length(
        plaintext_length
    );
    if (message_length < b64_output_length(raw_length)) {
        from_c(session)->last_error =
            olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    std::size_t result = from_c(session)->encrypt(
        from_c(plaintext), plaintext_length,
        from_c(random), random_length,
        b64_output_pos(from_c(message), raw_length), raw_length
    );
    olm::unset(random, random_length);
    if (result == std::size_t(-1)) {
        return result;
    }
    return b64_output(from_c(message), raw_length);
}


size_t olm_decrypt_max_plaintext_length(
    OlmSession * session,
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
        olm::MessageType(message_type), from_c(message), raw_length
    );
}


size_t olm_decrypt(
    OlmSession * session,
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
        olm::MessageType(message_type), from_c(message), raw_length,
        from_c(plaintext), max_plaintext_length
    );
}


size_t olm_sha256_length(
   OlmUtility * utility
) {
    return b64_output_length(from_c(utility)->sha256_length());
}


size_t olm_sha256(
    OlmUtility * utility,
    void const * input, size_t input_length,
    void * output, size_t output_length
) {
    std::size_t raw_length = from_c(utility)->sha256_length();
    if (output_length < b64_output_length(raw_length)) {
        from_c(utility)->last_error =
            olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    std::size_t result = from_c(utility)->sha256(
       from_c(input), input_length,
       b64_output_pos(from_c(output), raw_length), raw_length
    );
    if (result == std::size_t(-1)) {
        return result;
    }
    return b64_output(from_c(output), raw_length);
}


size_t olm_ed25519_verify(
    OlmUtility * utility,
    void const * key, size_t key_length,
    void const * message, size_t message_length,
    void * signature, size_t signature_length
) {
    if (olm::decode_base64_length(key_length) != olm::KEY_LENGTH) {
        from_c(utility)->last_error = olm::ErrorCode::INVALID_BASE64;
        return std::size_t(-1);
    }
    olm::Ed25519PublicKey verify_key;
    olm::decode_base64(from_c(key), key_length, verify_key.public_key);
    std::size_t raw_signature_length = b64_input(
        from_c(signature), signature_length, from_c(utility)->last_error
    );
    if (raw_signature_length == std::size_t(-1)) {
        return std::size_t(-1);
    }
    return from_c(utility)->ed25519_verify(
        verify_key,
        from_c(message), message_length,
        from_c(signature), raw_signature_length
    );
}

}
