/* Copyright 2015, 2016 OpenMarket Ltd
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
#include <ctime>
#include "olm/account.hh"
#include "olm/base64.hh"
#include "olm/pickle.h"
#include "olm/pickle.hh"
#include "olm/memory.hh"

olm::Account::Account(
) : next_prekey_id(0),
    last_prekey_publish_time(0),
    num_prekeys(0),
    num_fallback_keys(0),
    next_one_time_key_id(0),
    last_error(OlmErrorCode::OLM_SUCCESS) {
}


olm::OneTimeKey const * olm::Account::lookup_key(
    _olm_curve25519_public_key const & public_key
) {
    for (olm::OneTimeKey const & key : one_time_keys) {
        if (olm::array_equal(key.key.public_key.public_key, public_key.public_key)) {
            return &key;
        }
    }
    if (num_fallback_keys >= 1
            && olm::array_equal(
                current_fallback_key.key.public_key.public_key, public_key.public_key
            )
    ) {
        return &current_fallback_key;
    }
    if (num_fallback_keys >= 2
            && olm::array_equal(
                prev_fallback_key.key.public_key.public_key, public_key.public_key
            )
    ) {
        return &prev_fallback_key;
    }
    return 0;
}

std::size_t olm::Account::remove_key(
    _olm_curve25519_public_key const & public_key
) {
    OneTimeKey * i;
    for (i = one_time_keys.begin(); i != one_time_keys.end(); ++i) {
        if (olm::array_equal(i->key.public_key.public_key, public_key.public_key)) {
            std::uint32_t id = i->id;
            one_time_keys.erase(i);
            return id;
        }
    }
    // check if the key is a fallback key, to avoid returning an error, but
    // don't actually remove it
    if (num_fallback_keys >= 1
            && olm::array_equal(
                current_fallback_key.key.public_key.public_key, public_key.public_key
            )
    ) {
        return current_fallback_key.id;
    }
    if (num_fallback_keys >= 2
            && olm::array_equal(
                prev_fallback_key.key.public_key.public_key, public_key.public_key
            )
    ) {
        return prev_fallback_key.id;
    }
    return std::size_t(-1);
}

std::size_t olm::Account::new_account_random_length() const {
    return ED25519_RANDOM_LENGTH + CURVE25519_RANDOM_LENGTH + generate_prekey_random_length();
}

std::size_t olm::Account::new_account(
    uint8_t const * random, std::size_t random_length
) {
    if (random_length < new_account_random_length()) {
        last_error = OlmErrorCode::OLM_NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }

    _olm_crypto_ed25519_generate_key(random, &identity_keys.ed25519_key);
    random += ED25519_RANDOM_LENGTH;
    _olm_crypto_curve25519_generate_key(random, &identity_keys.curve25519_key);
    random += CURVE25519_RANDOM_LENGTH;
    generate_prekey(random, generate_prekey_random_length());

    return 0;
}

namespace {

uint8_t KEY_JSON_ED25519[] = "\"ed25519\":";
uint8_t KEY_JSON_CURVE25519[] = "\"curve25519\":";

template<typename T>
static std::uint8_t * write_string(
    std::uint8_t * pos,
    T const & value
) {
    std::memcpy(pos, value, sizeof(T) - 1);
    return pos + (sizeof(T) - 1);
}

}


std::size_t olm::Account::get_identity_json_length() const {
    std::size_t length = 0;
    length += 1; /* { */
    length += sizeof(KEY_JSON_CURVE25519) - 1;
    length += 1; /* " */
    length += olm::encode_base64_length(
        sizeof(identity_keys.curve25519_key.public_key)
    );
    length += 2; /* ", */
    length += sizeof(KEY_JSON_ED25519) - 1;
    length += 1; /* " */
    length += olm::encode_base64_length(
        sizeof(identity_keys.ed25519_key.public_key)
    );
    length += 2; /* "} */
    return length;
}


std::size_t olm::Account::get_identity_json(
    std::uint8_t * identity_json, std::size_t identity_json_length
) {
    std::uint8_t * pos = identity_json;
    size_t expected_length = get_identity_json_length();

    if (identity_json_length < expected_length) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    *(pos++) = '{';
    pos = write_string(pos, KEY_JSON_CURVE25519);
    *(pos++) = '\"';
    pos = olm::encode_base64(
        identity_keys.curve25519_key.public_key.public_key,
        sizeof(identity_keys.curve25519_key.public_key.public_key),
        pos
    );
    *(pos++) = '\"'; *(pos++) = ',';
    pos = write_string(pos, KEY_JSON_ED25519);
    *(pos++) = '\"';
    pos = olm::encode_base64(
        identity_keys.ed25519_key.public_key.public_key,
        sizeof(identity_keys.ed25519_key.public_key.public_key),
        pos
    );
    *(pos++) = '\"'; *(pos++) = '}';
    return pos - identity_json;
}


std::size_t olm::Account::signature_length(
) const {
    return ED25519_SIGNATURE_LENGTH;
}


std::size_t olm::Account::sign(
    std::uint8_t const * message, std::size_t message_length,
    std::uint8_t * signature, std::size_t signature_length
) {
    if (signature_length < this->signature_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    _olm_crypto_ed25519_sign(
        &identity_keys.ed25519_key, message, message_length, signature
    );
    return this->signature_length();
}


std::size_t olm::Account::get_one_time_keys_json_length(
) const {
    std::size_t length = 0;
    bool is_empty = true;
    for (auto const & key : one_time_keys) {
        if (key.published) {
            continue;
        }
        is_empty = false;
        length += 2; /* {" */
        length += olm::encode_base64_length(_olm_pickle_uint32_length(key.id));
        length += 3; /* ":" */
        length += olm::encode_base64_length(sizeof(key.key.public_key));
        length += 1; /* " */
    }
    if (is_empty) {
        length += 1; /* { */
    }
    length += 3; /* }{} */
    length += sizeof(KEY_JSON_CURVE25519) - 1;
    return length;
}


std::size_t olm::Account::get_one_time_keys_json(
    std::uint8_t * one_time_json, std::size_t one_time_json_length
) {
    std::uint8_t * pos = one_time_json;
    if (one_time_json_length < get_one_time_keys_json_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    *(pos++) = '{';
    pos = write_string(pos, KEY_JSON_CURVE25519);
    std::uint8_t sep = '{';
    for (auto const & key : one_time_keys) {
        if (key.published) {
            continue;
        }
        *(pos++) = sep;
        *(pos++) = '\"';
        std::uint8_t key_id[_olm_pickle_uint32_length(key.id)];
        _olm_pickle_uint32(key_id, key.id);
        pos = olm::encode_base64(key_id, sizeof(key_id), pos);
        *(pos++) = '\"'; *(pos++) = ':'; *(pos++) = '\"';
        pos = olm::encode_base64(
            key.key.public_key.public_key, sizeof(key.key.public_key.public_key), pos
        );
        *(pos++) = '\"';
        sep = ',';
    }
    if (sep != ',') {
        /* The list was empty */
        *(pos++) = sep;
    }
    *(pos++) = '}';
    *(pos++) = '}';
    return pos - one_time_json;
}


std::size_t olm::Account::mark_keys_as_published(
) {
    std::size_t count = 0;
    for (auto & key : one_time_keys) {
        if (!key.published) {
            key.published = true;
            count++;
        }
    }
    current_fallback_key.published = true;
    return count;
}


std::size_t olm::Account::max_number_of_one_time_keys(
) const {
    return olm::MAX_ONE_TIME_KEYS;
}

std::size_t olm::Account::generate_one_time_keys_random_length(
    std::size_t number_of_keys
) const {
    return CURVE25519_RANDOM_LENGTH * number_of_keys;
}

std::size_t olm::Account::generate_one_time_keys(
    std::size_t number_of_keys,
    std::uint8_t const * random, std::size_t random_length
) {
    if (random_length < generate_one_time_keys_random_length(number_of_keys)) {
        last_error = OlmErrorCode::OLM_NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }
    for (unsigned i = 0; i < number_of_keys; ++i) {
        OneTimeKey & key = *one_time_keys.insert(one_time_keys.begin());
        key.id = ++next_one_time_key_id;
        key.published = false;
        _olm_crypto_curve25519_generate_key(random, &key.key);
        random += CURVE25519_RANDOM_LENGTH;
    }
    return number_of_keys;
}

std::size_t olm::Account::generate_prekey_random_length() const {
    return CURVE25519_RANDOM_LENGTH;
}

std::size_t olm::Account::generate_prekey(
    std::uint8_t const * random, std::size_t random_length
) {
    if (random_length < generate_prekey_random_length()) {
        last_error = OlmErrorCode::OLM_NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }
    if (num_prekeys < 2) {
        num_prekeys++;
    }
    prev_prekey = current_prekey;
    current_prekey.id = ++next_prekey_id;
    current_prekey.published = false;
    _olm_crypto_curve25519_generate_key(random, &current_prekey.key);

    uint8_t message[CURVE25519_KEY_LENGTH + sizeof(current_prekey.id)];
    memcpy(message, current_prekey.key.public_key.public_key, CURVE25519_KEY_LENGTH);
    memcpy(message+CURVE25519_KEY_LENGTH, &current_prekey.id, sizeof(current_prekey.id));
    sign(current_prekey.key.public_key.public_key, CURVE25519_KEY_LENGTH,
    current_prekey.signature, signature_length());
    return 1;
}

std::size_t olm::Account::get_prekey_json_length(
) const {
    std::size_t length = 4 + sizeof(KEY_JSON_CURVE25519) - 1; /* {"curve25519":{}} */
    const PreKey & key = current_prekey;
    length += 1; /* " */
    length += olm::encode_base64_length(_olm_pickle_uint32_length(key.id));
    length += 3; /* ":" */
    length += olm::encode_base64_length(sizeof(key.key.public_key));
    length += 1; /* " */
    return length;
}

std::size_t olm::Account::get_prekey_json(
    std::uint8_t * prekey_json, std::size_t prekey_json_length
) {
    std::uint8_t * pos = prekey_json;
    if (prekey_json_length < get_prekey_json_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    *(pos++) = '{';
    pos = write_string(pos, KEY_JSON_CURVE25519);
    *(pos++) = '{';
    PreKey & key = current_prekey;
    *(pos++) = '\"';
    std::uint8_t key_id[_olm_pickle_uint32_length(key.id)];
    _olm_pickle_uint32(key_id, key.id);
    pos = olm::encode_base64(key_id, sizeof(key_id), pos);
    *(pos++) = '\"'; *(pos++) = ':'; *(pos++) = '\"';
    pos = olm::encode_base64(
    key.key.public_key.public_key, sizeof(key.key.public_key.public_key), pos
    );
    *(pos++) = '\"';
    *(pos++) = '}';
    *(pos++) = '}';
    return pos - prekey_json;
}

void olm::Account::forget_old_prekey(
) {
    if (num_prekeys == 2) {
        olm::unset(&prev_prekey, sizeof(prev_prekey));
        num_prekeys--;
    }
}

olm::PreKey const * olm::Account::lookup_prekey(
    _olm_curve25519_public_key const & public_key
) {
    if (olm::array_equal(current_prekey.key.public_key.public_key, public_key.public_key)) {
        return &current_prekey;
    } else if (olm::array_equal(prev_prekey.key.public_key.public_key, public_key.public_key)) {
        return &prev_prekey;
    }
    return 0;
}

std::size_t olm::Account::get_unpublished_prekey_json(
    std::uint8_t * prekey_json, std::size_t prekey_json_length) {
        std::uint8_t * pos = prekey_json;
    if (prekey_json_length < get_prekey_json_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    if (current_prekey.published) {
        return 0;
    }
    *(pos++) = '{';
    pos = write_string(pos, KEY_JSON_CURVE25519);
    *(pos++) = '{';
    PreKey & key = current_prekey;
    *(pos++) = '\"';
    std::uint8_t key_id[_olm_pickle_uint32_length(key.id)];
    _olm_pickle_uint32(key_id, key.id);
    pos = olm::encode_base64(key_id, sizeof(key_id), pos);
    *(pos++) = '\"'; *(pos++) = ':'; *(pos++) = '\"';
    pos = olm::encode_base64(
        key.key.public_key.public_key, sizeof(key.key.public_key.public_key), pos
    );
    *(pos++) = '\"';
    *(pos++) = '}';
    *(pos++) = '}';
    return pos - prekey_json;
}

std::size_t olm::Account::get_prekey_signature(
    std::uint8_t * signature) {
    olm::encode_base64(current_prekey.signature, signature_length(), signature);
    return signature_length();
}

std::uint64_t olm::Account::get_last_prekey_publish_time() {
    return last_prekey_publish_time;
}

std::size_t olm::Account::mark_prekey_as_published() {
    if (current_prekey.published) return std::size_t(-1);

    last_prekey_publish_time = std::time(nullptr);
    current_prekey.published = true;

    return 0;
}

std::size_t olm::Account::generate_fallback_key_random_length() const {
    return CURVE25519_RANDOM_LENGTH;
}

std::size_t olm::Account::generate_fallback_key(
    std::uint8_t const * random, std::size_t random_length
) {
    if (random_length < generate_fallback_key_random_length()) {
        last_error = OlmErrorCode::OLM_NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }
    if (num_fallback_keys < 2) {
        num_fallback_keys++;
    }
    prev_fallback_key = current_fallback_key;
    current_fallback_key.id = ++next_one_time_key_id;
    current_fallback_key.published = false;
    _olm_crypto_curve25519_generate_key(random, &current_fallback_key.key);
    return 1;
}


std::size_t olm::Account::get_fallback_key_json_length(
) const {
    std::size_t length = 4 + sizeof(KEY_JSON_CURVE25519) - 1; /* {"curve25519":{}} */
    if (num_fallback_keys >= 1) {
        const OneTimeKey & key = current_fallback_key;
        length += 1; /* " */
        length += olm::encode_base64_length(_olm_pickle_uint32_length(key.id));
        length += 3; /* ":" */
        length += olm::encode_base64_length(sizeof(key.key.public_key));
        length += 1; /* " */
    }
    return length;
}

std::size_t olm::Account::get_fallback_key_json(
    std::uint8_t * fallback_json, std::size_t fallback_json_length
) {
    std::uint8_t * pos = fallback_json;
    if (fallback_json_length < get_fallback_key_json_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    *(pos++) = '{';
    pos = write_string(pos, KEY_JSON_CURVE25519);
    *(pos++) = '{';
    OneTimeKey & key = current_fallback_key;
    if (num_fallback_keys >= 1) {
        *(pos++) = '\"';
        std::uint8_t key_id[_olm_pickle_uint32_length(key.id)];
        _olm_pickle_uint32(key_id, key.id);
        pos = olm::encode_base64(key_id, sizeof(key_id), pos);
        *(pos++) = '\"'; *(pos++) = ':'; *(pos++) = '\"';
        pos = olm::encode_base64(
            key.key.public_key.public_key, sizeof(key.key.public_key.public_key), pos
        );
        *(pos++) = '\"';
    }
    *(pos++) = '}';
    *(pos++) = '}';
    return pos - fallback_json;
}

std::size_t olm::Account::get_unpublished_fallback_key_json_length(
) const {
    std::size_t length = 4 + sizeof(KEY_JSON_CURVE25519) - 1; /* {"curve25519":{}} */
    const OneTimeKey & key = current_fallback_key;
    if (num_fallback_keys >= 1 && !key.published) {
        length += 1; /* " */
        length += olm::encode_base64_length(_olm_pickle_uint32_length(key.id));
        length += 3; /* ":" */
        length += olm::encode_base64_length(sizeof(key.key.public_key));
        length += 1; /* " */
    }
    return length;
}

std::size_t olm::Account::get_unpublished_fallback_key_json(
    std::uint8_t * fallback_json, std::size_t fallback_json_length
) {
    std::uint8_t * pos = fallback_json;
    if (fallback_json_length < get_unpublished_fallback_key_json_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    *(pos++) = '{';
    pos = write_string(pos, KEY_JSON_CURVE25519);
    *(pos++) = '{';
    OneTimeKey & key = current_fallback_key;
    if (num_fallback_keys >= 1 && !key.published) {
        *(pos++) = '\"';
        std::uint8_t key_id[_olm_pickle_uint32_length(key.id)];
        _olm_pickle_uint32(key_id, key.id);
        pos = olm::encode_base64(key_id, sizeof(key_id), pos);
        *(pos++) = '\"'; *(pos++) = ':'; *(pos++) = '\"';
        pos = olm::encode_base64(
            key.key.public_key.public_key, sizeof(key.key.public_key.public_key), pos
        );
        *(pos++) = '\"';
    }
    *(pos++) = '}';
    *(pos++) = '}';
    return pos - fallback_json;
}

void olm::Account::forget_old_fallback_key(
) {
    if (num_fallback_keys >= 2) {
        num_fallback_keys = 1;
        olm::unset(&prev_fallback_key, sizeof(prev_fallback_key));
    }
}

namespace olm {

static std::size_t pickle_length(
    olm::IdentityKeys const & value
) {
    size_t length = 0;
    length += _olm_pickle_ed25519_key_pair_length(&value.ed25519_key);
    length += olm::pickle_length(value.curve25519_key);
    return length;
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    olm::IdentityKeys const & value
) {
    pos = _olm_pickle_ed25519_key_pair(pos, &value.ed25519_key);
    pos = olm::pickle(pos, value.curve25519_key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::IdentityKeys & value
) {
    pos = _olm_unpickle_ed25519_key_pair(pos, end, &value.ed25519_key); UNPICKLE_OK(pos);
    pos = olm::unpickle(pos, end, value.curve25519_key); UNPICKLE_OK(pos);
    return pos;
}

static std::size_t pickle_length(
    olm::PreKey const & value
) {
    std::size_t length = 0;
    length += olm::pickle_length(value.id);
    length += olm::pickle_length(value.published);
    length += olm::pickle_length(value.key);
    length += ED25519_SIGNATURE_LENGTH;
    return length;
}

static std::uint8_t * pickle(
    std::uint8_t * pos,
    olm::PreKey const & value
) {
    pos = olm::pickle(pos, value.id);
    pos = olm::pickle(pos, value.published);
    pos = olm::pickle(pos, value.key);
    pos = olm::pickle_bytes(pos, value.signature, ED25519_SIGNATURE_LENGTH);
    return pos;
}

static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::PreKey & value
) {
    pos = olm::unpickle(pos, end, value.id); UNPICKLE_OK(pos);
    pos = olm::unpickle(pos, end, value.published); UNPICKLE_OK(pos);
    pos = olm::unpickle(pos, end, value.key); UNPICKLE_OK(pos);
    pos = olm::unpickle_bytes(pos, end, value.signature, ED25519_SIGNATURE_LENGTH); UNPICKLE_OK(pos);
    return pos;
}


static std::size_t pickle_length(
    olm::OneTimeKey const & value
) {
    std::size_t length = 0;
    length += olm::pickle_length(value.id);
    length += olm::pickle_length(value.published);
    length += olm::pickle_length(value.key);
    return length;
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    olm::OneTimeKey const & value
) {
    pos = olm::pickle(pos, value.id);
    pos = olm::pickle(pos, value.published);
    pos = olm::pickle(pos, value.key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::OneTimeKey & value
) {
    pos = olm::unpickle(pos, end, value.id); UNPICKLE_OK(pos);
    pos = olm::unpickle(pos, end, value.published); UNPICKLE_OK(pos);
    pos = olm::unpickle(pos, end, value.key); UNPICKLE_OK(pos);
    return pos;
}

} // namespace olm

namespace {
// pickle version 1 used only 32 bytes for the ed25519 private key.
// Any keys thus used should be considered compromised.
// pickle version 2 does not have fallback keys.
// pickle version 3 does not store whether the current fallback key is published.
// pickle version 4 does not use X3DH.
static const std::uint32_t ACCOUNT_PICKLE_VERSION = 10005;
}


std::size_t olm::pickle_length(
    olm::Account const & value
) {
    std::size_t length = 0;
    length += olm::pickle_length(ACCOUNT_PICKLE_VERSION);
    length += olm::pickle_length(value.identity_keys);
    length += olm::pickle_length(value.num_prekeys);
    if (value.num_prekeys >= 1) {
        length += olm::pickle_length(value.current_prekey);
        if (value.num_prekeys >= 2) {
            length += olm::pickle_length(value.prev_prekey);
        }
    }
    length += olm::pickle_length(value.next_prekey_id);
    length += olm::pickle_length(value.last_prekey_publish_time);
    length += olm::pickle_length(value.one_time_keys);
    length += olm::pickle_length(value.num_fallback_keys);
    if (value.num_fallback_keys >= 1) {
        length += olm::pickle_length(value.current_fallback_key);
        if (value.num_fallback_keys >= 2) {
            length += olm::pickle_length(value.prev_fallback_key);
        }
    }
    length += olm::pickle_length(value.next_one_time_key_id);
    return length;
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    olm::Account const & value
) {
    pos = olm::pickle(pos, ACCOUNT_PICKLE_VERSION);
    pos = olm::pickle(pos, value.identity_keys);
    pos = olm::pickle(pos, value.num_prekeys);
    if (value.num_prekeys >= 1) {
        pos = olm::pickle(pos, value.current_prekey);
        if (value.num_prekeys >= 2) {
            pos = olm::pickle(pos, value.prev_prekey);
        }
    }
    pos = olm::pickle(pos, value.next_prekey_id);
    pos = olm::pickle(pos, value.last_prekey_publish_time);
    pos = olm::pickle(pos, value.one_time_keys);
    pos = olm::pickle(pos, value.num_fallback_keys);
    if (value.num_fallback_keys >= 1) {
        pos = olm::pickle(pos, value.current_fallback_key);
        if (value.num_fallback_keys >= 2) {
            pos = olm::pickle(pos, value.prev_fallback_key);
        }
    }
    pos = olm::pickle(pos, value.next_one_time_key_id);
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Account & value
) {
    uint32_t pickle_version;

    pos = olm::unpickle(pos, end, pickle_version); UNPICKLE_OK(pos);

    switch (pickle_version) {
        case ACCOUNT_PICKLE_VERSION:
        case 4:
        case 3:
        case 2:
            break;
        case 1:
            value.last_error = OlmErrorCode::OLM_BAD_LEGACY_ACCOUNT_PICKLE;
            return nullptr;
        default:
            value.last_error = OlmErrorCode::OLM_UNKNOWN_PICKLE_VERSION;
            return nullptr;
    }

    pos = olm::unpickle(pos, end, value.identity_keys); UNPICKLE_OK(pos);
    if (pickle_version >= 10005) {
        // version 10005 adds support for X3DH
        pos = olm::unpickle(pos, end, value.num_prekeys); UNPICKLE_OK(pos);
        if (value.num_prekeys >= 1) {
            pos = olm::unpickle(pos, end, value.current_prekey); UNPICKLE_OK(pos);
            if (value.num_prekeys >= 2) {
                pos = olm::unpickle(pos, end, value.prev_prekey); UNPICKLE_OK(pos);
                if (value.num_prekeys >= 3) {
                    value.last_error = OlmErrorCode::OLM_CORRUPTED_PICKLE;
                    return nullptr;
                }
            }
        }
        pos = olm::unpickle(pos, end, value.next_prekey_id); UNPICKLE_OK(pos);
        pos = olm::unpickle(pos, end, value.last_prekey_publish_time); UNPICKLE_OK(pos);
    }
    pos = olm::unpickle(pos, end, value.one_time_keys); UNPICKLE_OK(pos);

    if (pickle_version <= 2) {
        // version 2 did not have fallback keys
        value.num_fallback_keys = 0;
    } else if (pickle_version == 3) {
        // version 3 used the published flag to indicate how many fallback keys
        // were present (we'll have to assume that the keys were published)
        pos = olm::unpickle(pos, end, value.current_fallback_key); UNPICKLE_OK(pos);
        pos = olm::unpickle(pos, end, value.prev_fallback_key); UNPICKLE_OK(pos);
        if (value.current_fallback_key.published) {
            if (value.prev_fallback_key.published) {
                value.num_fallback_keys = 2;
            } else {
                value.num_fallback_keys = 1;
            }
        } else  {
            value.num_fallback_keys = 0;
        }
    } else {
        pos = olm::unpickle(pos, end, value.num_fallback_keys); UNPICKLE_OK(pos);
        if (value.num_fallback_keys >= 1) {
            pos = olm::unpickle(pos, end, value.current_fallback_key); UNPICKLE_OK(pos);
            if (value.num_fallback_keys >= 2) {
                pos = olm::unpickle(pos, end, value.prev_fallback_key); UNPICKLE_OK(pos);
                if (value.num_fallback_keys >= 3) {
                    value.last_error = OlmErrorCode::OLM_CORRUPTED_PICKLE;
                    return nullptr;
                }
            }
        }
    }

    pos = olm::unpickle(pos, end, value.next_one_time_key_id); UNPICKLE_OK(pos);
    return pos;
}
