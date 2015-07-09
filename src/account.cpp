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
#include "olm/account.hh"
#include "olm/base64.hh"
#include "olm/pickle.hh"

olm::Account::Account(
) : next_one_time_key_id(0),
    last_error(olm::ErrorCode::SUCCESS) {
}


olm::OneTimeKey const * olm::Account::lookup_key(
    olm::Curve25519PublicKey const & public_key
) {
    for (olm::OneTimeKey const & key : one_time_keys) {
        if (0 == memcmp(key.key.public_key, public_key.public_key, 32)) {
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
        if (0 == memcmp(i->key.public_key, public_key.public_key, 32)) {
            std::uint32_t id = i->id;
            one_time_keys.erase(i);
            return id;
        }
    }
    return std::size_t(-1);
}

std::size_t olm::Account::new_account_random_length() {
    return 12 * 32;
}

std::size_t olm::Account::new_account(
    uint8_t const * random, std::size_t random_length
) {
    if (random_length < new_account_random_length()) {
        last_error = olm::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }

    olm::ed25519_generate_key(random, identity_keys.ed25519_key);
    random += 32;
    olm::curve25519_generate_key(random, identity_keys.curve25519_key);
    random += 32;

    generate_one_time_keys(10, random, random_length - 64);

    return 0;
}

namespace {

static const uint8_t IDENTITY_JSON_PART_0[] =
    "{\"algorithms\":"
    "[\"m.olm.curve25519-aes-sha256\""
    "],\"device_id\":\"";
static const uint8_t IDENTITY_JSON_PART_1[] = "\",\"keys\":{\"curve25519:";
static const uint8_t IDENTITY_JSON_PART_2[] = "\":\"";
static const uint8_t IDENTITY_JSON_PART_3[] = "\",\"ed25519:";
static const uint8_t IDENTITY_JSON_PART_4[] = "\":\"";
static const uint8_t IDENTITY_JSON_PART_5[] = "\"},\"user_id\":\"";
static const uint8_t IDENTITY_JSON_PART_6[] = "\",\"valid_after_ts\":";
static const uint8_t IDENTITY_JSON_PART_7[] = ",\"valid_until_ts\":";
static const uint8_t IDENTITY_JSON_PART_8[] = ",\"signatures\":{\"";
static const uint8_t IDENTITY_JSON_PART_9[] = "/";
static const uint8_t IDENTITY_JSON_PART_A[] = "\":{\"ed25519:";
static const uint8_t IDENTITY_JSON_PART_B[] = "\":\"";
static const uint8_t IDENTITY_JSON_PART_C[] = "\"}}}";

std::size_t count_digits(
    std::uint64_t value
) {
    std::size_t digits = 0;
    do {
        digits++;
        value /= 10;
    } while (value);
    return digits;
}

template<typename T>
std::uint8_t * write_string(
    std::uint8_t * pos,
    T const & value
) {
    std::memcpy(pos, value, sizeof(T) - 1);
    return pos + (sizeof(T) - 1);
}

std::uint8_t * write_string(
    std::uint8_t * pos,
    std::uint8_t const * value, std::size_t value_length
) {
    std::memcpy(pos, value, value_length);
    return pos + value_length;
}

std::uint8_t * write_digits(
    std::uint8_t * pos,
    std::uint64_t value
) {
    size_t digits = count_digits(value);
    pos += digits;
    do {
        *(--pos) = '0' + (value % 10);
        value /= 10;
    } while (value);
    return pos + digits;
}

}


std::size_t olm::Account::get_identity_json_length(
    std::size_t user_id_length,
    std::size_t device_id_length,
    std::uint64_t valid_after_ts,
    std::uint64_t valid_until_ts
) {
    std::size_t length = 0;
    length += sizeof(IDENTITY_JSON_PART_0) - 1;
    length += device_id_length;
    length += sizeof(IDENTITY_JSON_PART_1) - 1;
    length += olm::encode_base64_length(3);
    length += sizeof(IDENTITY_JSON_PART_2) - 1;
    length += olm::encode_base64_length(
        sizeof(identity_keys.curve25519_key.public_key)
    );
    length += sizeof(IDENTITY_JSON_PART_3) - 1;
    length += olm::encode_base64_length(3);
    length += sizeof(IDENTITY_JSON_PART_4) - 1;
    length += olm::encode_base64_length(
        sizeof(identity_keys.ed25519_key.public_key)
    );
    length += sizeof(IDENTITY_JSON_PART_5) - 1;
    length += user_id_length;
    length += sizeof(IDENTITY_JSON_PART_6) - 1;
    length += count_digits(valid_after_ts);
    length += sizeof(IDENTITY_JSON_PART_7) - 1;
    length += count_digits(valid_until_ts);
    length += sizeof(IDENTITY_JSON_PART_8) - 1;
    length += user_id_length;
    length += sizeof(IDENTITY_JSON_PART_9) - 1;
    length += device_id_length;
    length += sizeof(IDENTITY_JSON_PART_A) - 1;
    length += olm::encode_base64_length(3);
    length += sizeof(IDENTITY_JSON_PART_B) - 1;
    length += olm::encode_base64_length(64);
    length += sizeof(IDENTITY_JSON_PART_C) - 1;
    return length;
}


std::size_t olm::Account::get_identity_json(
    std::uint8_t const * user_id, std::size_t user_id_length,
    std::uint8_t const * device_id, std::size_t device_id_length,
    std::uint64_t valid_after_ts,
    std::uint64_t valid_until_ts,
    std::uint8_t * identity_json, std::size_t identity_json_length
) {
    std::uint8_t * pos = identity_json;
    std::uint8_t signature[64];
    size_t expected_length = get_identity_json_length(
            user_id_length, device_id_length, valid_after_ts, valid_until_ts
    );

    if (identity_json_length < expected_length) {
        last_error = olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }

    pos = write_string(pos, IDENTITY_JSON_PART_0);
    pos = write_string(pos, device_id, device_id_length);
    pos = write_string(pos, IDENTITY_JSON_PART_1);
    pos = encode_base64(identity_keys.curve25519_key.public_key, 3, pos);
    pos = write_string(pos, IDENTITY_JSON_PART_2);
    pos = encode_base64(identity_keys.curve25519_key.public_key, 32, pos);
    pos = write_string(pos, IDENTITY_JSON_PART_3);
    pos = encode_base64(identity_keys.ed25519_key.public_key, 3, pos);
    pos = write_string(pos, IDENTITY_JSON_PART_4);
    pos = encode_base64(identity_keys.ed25519_key.public_key, 32, pos);
    pos = write_string(pos, IDENTITY_JSON_PART_5);
    pos = write_string(pos, user_id, user_id_length);
    pos = write_string(pos, IDENTITY_JSON_PART_6);
    pos = write_digits(pos, valid_after_ts);
    pos = write_string(pos, IDENTITY_JSON_PART_7);
    pos = write_digits(pos, valid_until_ts);
    *pos = '}';
    // Sign the JSON up to written up to this point.
    ed25519_sign(
        identity_keys.ed25519_key,
        identity_json, 1 + pos - identity_json,
        signature
    );
    // Append the signature to the end of the JSON.
    pos = write_string(pos, IDENTITY_JSON_PART_8);
    pos = write_string(pos, user_id, user_id_length);
    pos = write_string(pos, IDENTITY_JSON_PART_9);
    pos = write_string(pos, device_id, device_id_length);
    pos = write_string(pos, IDENTITY_JSON_PART_A);
    pos = encode_base64(identity_keys.ed25519_key.public_key, 3, pos);
    pos = write_string(pos, IDENTITY_JSON_PART_B);
    pos = encode_base64(signature, 64, pos);
    pos = write_string(pos, IDENTITY_JSON_PART_C);
    return pos - identity_json;
}

namespace {
uint8_t ONE_TIME_KEY_JSON_ALG[] = "curve25519";
}

std::size_t olm::Account::get_one_time_keys_json_length(
) {
    std::size_t length = 0;
    for (auto const & key : one_time_keys) {
        if (key.published) {
            continue;
        }
        length += 2; /* {" */
        length += sizeof(ONE_TIME_KEY_JSON_ALG) - 1;
        length += 1; /* : */
        length += olm::encode_base64_length(olm::pickle_length(key.id));
        length += 3; /* ":" */
        length += olm::encode_base64_length(sizeof(key.key.public_key));
        length += 1; /* " */
    }
    if (length) {
        return length + 1; /* } */
    } else {
        return 2; /* {} */
    }
}


std::size_t olm::Account::get_one_time_keys_json(
    std::uint8_t * one_time_json, std::size_t one_time_json_length
) {
    std::uint8_t * pos = one_time_json;
    if (one_time_json_length < get_one_time_keys_json_length()) {
        last_error = olm::ErrorCode::OUTPUT_BUFFER_TOO_SMALL;
        return std::size_t(-1);
    }
    std::uint8_t sep = '{';
    for (auto const & key : one_time_keys) {
        if (key.published) {
            continue;
        }
        *(pos++) = sep;
        *(pos++) = '\"';
        pos = write_string(pos, ONE_TIME_KEY_JSON_ALG);
        *(pos++) = ':';
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
        *(pos++) = sep;
    }
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
    return 32 * number_of_keys;
}

std::size_t olm::Account::generate_one_time_keys(
    std::size_t number_of_keys,
    std::uint8_t const * random, std::size_t random_length
) {
    if (random_length < generate_one_time_keys_random_length(number_of_keys)) {
        last_error = olm::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }
    for (unsigned i = 0; i < number_of_keys; ++i) {
        OneTimeKey & key = *one_time_keys.insert(one_time_keys.begin());
        key.id = ++next_one_time_key_id;
        key.published = false;
        olm::curve25519_generate_key(random, key.key);
        random += 32;
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


std::size_t olm::pickle_length(
    olm::Account const & value
) {
    std::size_t length = 0;
    length += olm::pickle_length(value.identity_keys);
    length += olm::pickle_length(value.one_time_keys);
    length += olm::pickle_length(value.next_one_time_key_id);
    return length;
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    olm::Account const & value
) {
    pos = olm::pickle(pos, value.identity_keys);
    pos = olm::pickle(pos, value.one_time_keys);
    pos = olm::pickle(pos, value.next_one_time_key_id);
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Account & value
) {
    pos = olm::unpickle(pos, end, value.identity_keys);
    pos = olm::unpickle(pos, end, value.one_time_keys);
    pos = olm::unpickle(pos, end, value.next_one_time_key_id);
    return pos;
}
