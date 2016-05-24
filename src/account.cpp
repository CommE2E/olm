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
#include "olm/account.hh"
#include "olm/base64.hh"
#include "olm/pickle.hh"
#include "olm/memory.hh"

olm::Account::Account(
) : next_one_time_key_id(0),
    last_error(OlmErrorCode::OLM_SUCCESS) {
}


olm::OneTimeKey const * olm::Account::lookup_key(
    olm::Curve25519PublicKey const & public_key
) {
    for (olm::OneTimeKey const & key : one_time_keys) {
        if (olm::array_equal(key.key.public_key, public_key.public_key)) {
            return &key;
        }
    }
    return 0;
}

std::size_t olm::Account::remove_key(
    olm::Curve25519PublicKey const & public_key
) {
    OneTimeKey * i;
    for (i = one_time_keys.begin(); i != one_time_keys.end(); ++i) {
        if (olm::array_equal(i->key.public_key, public_key.public_key)) {
            std::uint32_t id = i->id;
            one_time_keys.erase(i);
            return id;
        }
    }
    return std::size_t(-1);
}

std::size_t olm::Account::new_account_random_length() {
    return 2 * olm::KEY_LENGTH;
}

std::size_t olm::Account::new_account(
    uint8_t const * random, std::size_t random_length
) {
    if (random_length < new_account_random_length()) {
        last_error = OlmErrorCode::OLM_NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }

    olm::ed25519_generate_key(random, identity_keys.ed25519_key);
    random += KEY_LENGTH;
    olm::curve25519_generate_key(random, identity_keys.curve25519_key);

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


std::size_t olm::Account::get_identity_json_length() {
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
        identity_keys.curve25519_key.public_key,
        sizeof(identity_keys.curve25519_key.public_key),
        pos
    );
    *(pos++) = '\"'; *(pos++) = ',';
    pos = write_string(pos, KEY_JSON_ED25519);
    *(pos++) = '\"';
    pos = olm::encode_base64(
        identity_keys.ed25519_key.public_key,
        sizeof(identity_keys.ed25519_key.public_key),
        pos
    );
    *(pos++) = '\"'; *(pos++) = '}';
    return pos - identity_json;
}


std::size_t olm::Account::signature_length(
) {
    return olm::SIGNATURE_LENGTH;
}


std::size_t olm::Account::sign(
    std::uint8_t const * message, std::size_t message_length,
    std::uint8_t * signature, std::size_t signature_length
) {
    if (signature_length < this->signature_length()) {
        last_error = OlmErrorCode::OLM_OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    olm::ed25519_sign(
        identity_keys.ed25519_key, message, message_length, signature
    );
    return this->signature_length();
}


std::size_t olm::Account::get_one_time_keys_json_length(
) {
    std::size_t length = 0;
    bool is_empty = true;
    for (auto const & key : one_time_keys) {
        if (key.published) {
            continue;
        }
        is_empty = false;
        length += 2; /* {" */
        length += olm::encode_base64_length(olm::pickle_length(key.id));
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
        std::uint8_t key_id[olm::pickle_length(key.id)];
        olm::pickle(key_id, key.id);
        pos = olm::encode_base64(key_id, sizeof(key_id), pos);
        *(pos++) = '\"'; *(pos++) = ':'; *(pos++) = '\"';
        pos = olm::encode_base64(
            key.key.public_key, sizeof(key.key.public_key), pos
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
    return count;
}


std::size_t olm::Account::max_number_of_one_time_keys(
) {
    return olm::MAX_ONE_TIME_KEYS;
}

std::size_t olm::Account::generate_one_time_keys_random_length(
    std::size_t number_of_keys
) {
    return olm::KEY_LENGTH * number_of_keys;
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
        olm::curve25519_generate_key(random, key.key);
        random += olm::KEY_LENGTH;
    }
    return number_of_keys;
}

namespace olm {

static std::size_t pickle_length(
    olm::IdentityKeys const & value
) {
    size_t length = 0;
    length += olm::pickle_length(value.ed25519_key);
    length += olm::pickle_length(value.curve25519_key);
    return length;
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    olm::IdentityKeys const & value
) {
    pos = olm::pickle(pos, value.ed25519_key);
    pos = olm::pickle(pos, value.curve25519_key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::IdentityKeys & value
) {
    pos = olm::unpickle(pos, end, value.ed25519_key);
    pos = olm::unpickle(pos, end, value.curve25519_key);
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
    pos = olm::unpickle(pos, end, value.id);
    pos = olm::unpickle(pos, end, value.published);
    pos = olm::unpickle(pos, end, value.key);
    return pos;
}

} // namespace olm

namespace {
static const std::uint32_t ACCOUNT_PICKLE_VERSION = 1;
}


std::size_t olm::pickle_length(
    olm::Account const & value
) {
    std::size_t length = 0;
    length += olm::pickle_length(ACCOUNT_PICKLE_VERSION);
    length += olm::pickle_length(value.identity_keys);
    length += olm::pickle_length(value.one_time_keys);
    length += olm::pickle_length(value.next_one_time_key_id);
    return length;
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    olm::Account const & value
) {
    pos = olm::pickle(pos, ACCOUNT_PICKLE_VERSION);
    pos = olm::pickle(pos, value.identity_keys);
    pos = olm::pickle(pos, value.one_time_keys);
    pos = olm::pickle(pos, value.next_one_time_key_id);
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Account & value
) {
    uint32_t pickle_version;
    pos = olm::unpickle(pos, end, pickle_version);
    if (pickle_version != ACCOUNT_PICKLE_VERSION) {
        value.last_error = OlmErrorCode::OLM_UNKNOWN_PICKLE_VERSION;
        return end;
    }
    pos = olm::unpickle(pos, end, value.identity_keys);
    pos = olm::unpickle(pos, end, value.one_time_keys);
    pos = olm::unpickle(pos, end, value.next_one_time_key_id);
    return pos;
}
