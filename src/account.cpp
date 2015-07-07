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


olm::OneTimeKey const * olm::Account::lookup_key(
    std::uint32_t id
) {
    for (olm::OneTimeKey const & key : one_time_keys) {
        if (key.id == id) return &key;
    }
    return 0;
}

std::size_t olm::Account::remove_key(
    std::uint32_t id
) {
    OneTimeKey * i;
    for (i = one_time_keys.begin(); i != one_time_keys.end(); ++i) {
        if (i->id == id) {
            one_time_keys.erase(i);
            return id;
        }
    }
    return std::size_t(-1);
}

std::size_t olm::Account::new_account_random_length() {
    return 103 * 32;
}

std::size_t olm::Account::new_account(
    uint8_t const * random, std::size_t random_length
) {
    if (random_length < new_account_random_length()) {
        last_error = olm::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }

    unsigned id = 0;

    olm::ed25519_generate_key(random, identity_keys.ed25519_key);
    random += 32;
    olm::curve25519_generate_key(random, identity_keys.curve25519_key);
    random += 32;

    for (unsigned i = 0; i < 10; ++i) {
        OneTimeKey & key = *one_time_keys.insert(one_time_keys.end());
        key.id = ++id;
        olm::curve25519_generate_key(random, key.key);
        random += 32;
    }

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
    length += 4;
    length += sizeof(IDENTITY_JSON_PART_2) - 1;
    length += 43;
    length += sizeof(IDENTITY_JSON_PART_3) - 1;
    length += 4;
    length += sizeof(IDENTITY_JSON_PART_4) - 1;
    length += 43;
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
    length += 4;
    length += sizeof(IDENTITY_JSON_PART_B) - 1;
    length += 86;
    length += sizeof(IDENTITY_JSON_PART_C) - 1;
    return length;
}


std::size_t olm::Account::get_identity_json(
    std::uint8_t const * user_id, std::size_t user_id_length,
    std::uint8_t const * device_id, std::size_t device_id_length,
    std::uint64_t valid_until_ts,
    std::uint64_t valid_after_ts,
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
    encode_base64(identity_keys.curve25519_key.public_key, 3, pos);
    pos += 4;
    pos = write_string(pos, IDENTITY_JSON_PART_2);
    encode_base64(identity_keys.curve25519_key.public_key, 32, pos);
    pos += 43;
    pos = write_string(pos, IDENTITY_JSON_PART_3);
    encode_base64(identity_keys.ed25519_key.public_key, 3, pos);
    pos += 4;
    pos = write_string(pos, IDENTITY_JSON_PART_4);
    encode_base64(identity_keys.ed25519_key.public_key, 32, pos);
    pos += 43;
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
    encode_base64(identity_keys.ed25519_key.public_key, 3, pos);
    pos += 4;
    pos = write_string(pos, IDENTITY_JSON_PART_B);
    encode_base64(signature, 64, pos);
    pos += 86;
    pos = write_string(pos, IDENTITY_JSON_PART_C);
    return pos - identity_json;
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
    return olm::pickle_length(value.id) + olm::pickle_length(value.key);
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    olm::OneTimeKey const & value
) {
    pos = olm::pickle(pos, value.id);
    pos = olm::pickle(pos, value.key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::OneTimeKey & value
) {
    pos = olm::unpickle(pos, end, value.id);
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
    return length;
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    olm::Account const & value
) {
    pos = olm::pickle(pos, value.identity_keys);
    pos = olm::pickle(pos, value.one_time_keys);
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Account & value
) {
    pos = olm::unpickle(pos, end, value.identity_keys);
    pos = olm::unpickle(pos, end, value.one_time_keys);
    return pos;
}
