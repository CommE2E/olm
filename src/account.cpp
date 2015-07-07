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
#include "olm/pickle.hh"


olm::LocalKey const * olm::Account::lookup_key(
    std::uint32_t id
) {
    for (olm::LocalKey const & key : one_time_keys) {
        if (key.id == id) return &key;
    }
    return 0;
}

std::size_t olm::Account::remove_key(
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

    identity_key.id = ++id;
    olm::curve25519_generate_key(random, identity_key.key);
    random += 32;

    random += 32;

    for (unsigned i = 0; i < 10; ++i) {
        LocalKey & key = *one_time_keys.insert(one_time_keys.end());
        key.id = ++id;
        olm::curve25519_generate_key(random, key.key);
        random += 32;
    }

    return 0;
}


namespace olm {


static std::size_t pickle_length(
    olm::LocalKey const & value
) {
    return olm::pickle_length(value.id) + olm::pickle_length(value.key);
}


static std::uint8_t * pickle(
    std::uint8_t * pos,
    olm::LocalKey const & value
) {
    pos = olm::pickle(pos, value.id);
    pos = olm::pickle(pos, value.key);
    return pos;
}


static std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::LocalKey & value
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
    length += olm::pickle_length(value.identity_key);
    length += olm::pickle_length(value.one_time_keys);
    return length;
}


std::uint8_t * olm::pickle(
    std::uint8_t * pos,
    olm::Account const & value
) {
    pos = olm::pickle(pos, value.identity_key);
    pos = olm::pickle(pos, value.one_time_keys);
    return pos;
}


std::uint8_t const * olm::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    olm::Account & value
) {
    pos = olm::unpickle(pos, end, value.identity_key);
    pos = olm::unpickle(pos, end, value.one_time_keys);
    return pos;
}
