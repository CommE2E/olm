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
#include "axolotl/account.hh"
#include "axolotl/pickle.hh"


axolotl::LocalKey const * axolotl::Account::lookup_key(
    std::uint32_t id
) {
    for (axolotl::LocalKey const & key : one_time_keys) {
        if (key.id == id) return &key;
    }
    return 0;
}

std::size_t axolotl::Account::remove_key(
    std::uint32_t id
) {
    LocalKey * i;
    for (i = one_time_keys.begin(); i != one_time_keys.end(); ++i) {
        if (i->id == id) {
            one_time_keys.erase(i);
            return id;
        }
    }
    return std::size_t(-1);
}

std::size_t axolotl::Account::new_account_random_length() {
    return 103 * 32;
}

std::size_t axolotl::Account::new_account(
    uint8_t const * random, std::size_t random_length
) {
    if (random_length < new_account_random_length()) {
        last_error = axolotl::ErrorCode::NOT_ENOUGH_RANDOM;
        return std::size_t(-1);
    }

    unsigned id = 0;

    identity_key.id = ++id;
    axolotl::generate_key(random, identity_key.key);
    random += 32;

    random += 32;

    last_resort_one_time_key.id = ++id;
    axolotl::generate_key(random, last_resort_one_time_key.key);
    random += 32;

    for (unsigned i = 0; i < 10; ++i) {
        LocalKey & key = *one_time_keys.insert(one_time_keys.end());
        key.id = ++id;
        axolotl::generate_key(random, key.key);
        random += 32;
    }

    return 0;
}


namespace axolotl {


static std::size_t pickle_length(
    axolotl::LocalKey const & value
) {
    return axolotl::pickle_length(value.id) + axolotl::pickle_length(value.key);
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    axolotl::LocalKey const & value
) {
    pos = axolotl::pickle(pos, value.id);
    pos = axolotl::pickle(pos, value.key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::LocalKey & value
) {
    pos = axolotl::unpickle(pos, end, value.id);
    pos = axolotl::unpickle(pos, end, value.key);
    return pos;
}


static std::size_t pickle_length(
    axolotl::SignedKey const & value
) {
    return axolotl::pickle_length((axolotl::LocalKey const &) value) + 64;
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    axolotl::SignedKey const & value
) {
    pos = axolotl::pickle(pos, (axolotl::LocalKey const &) value);
    pos = axolotl::pickle_bytes(pos, value.signature, 64);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::SignedKey & value
) {
    pos = axolotl::unpickle(pos, end, (axolotl::LocalKey &) value);
    pos = axolotl::unpickle_bytes(pos, end, value.signature, 64);
    return pos;
}


} // namespace axolotl


std::size_t axolotl::pickle_length(
    axolotl::Account const & value
) {
    std::size_t length = 0;
    length += axolotl::pickle_length(value.identity_key);
    length += axolotl::pickle_length(value.last_resort_one_time_key);
    length += axolotl::pickle_length(value.one_time_keys);
    return length;
}


std::uint8_t * axolotl::pickle(
    std::uint8_t * pos,
    axolotl::Account const & value
) {
    pos = axolotl::pickle(pos, value.identity_key);
    pos = axolotl::pickle(pos, value.last_resort_one_time_key);
    pos = axolotl::pickle(pos, value.one_time_keys);
    return pos;
}


std::uint8_t const * axolotl::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::Account & value
) {
    pos = axolotl::unpickle(pos, end, value.identity_key);
    pos = axolotl::unpickle(pos, end, value.last_resort_one_time_key);
    pos = axolotl::unpickle(pos, end, value.one_time_keys);
    return pos;
}
