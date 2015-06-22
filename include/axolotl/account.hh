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
#ifndef AXOLOTL_ACCOUNT_HH_
#define AXOLOTL_ACCOUNT_HH_

#include "axolotl/list.hh"
#include "axolotl/crypto.hh"
#include "axolotl/error.hh"

#include <cstdint>

namespace axolotl {


struct LocalKey {
    std::uint32_t id;
    Curve25519KeyPair key;
};


struct SignedKey : LocalKey {
    std::uint8_t signature[64];
};


static std::size_t const MAX_ONE_TIME_KEYS = 100;

struct Account {
    LocalKey identity_key;
    LocalKey last_resort_one_time_key;
    List<LocalKey, MAX_ONE_TIME_KEYS> one_time_keys;
    ErrorCode last_error;

    /** Number of random bytes needed to create a new account */
    std::size_t new_account_random_length();

    /** Create a new account. Returns NOT_ENOUGH_RANDOM if the number of random
     * bytes is too small. */
    std::size_t new_account(
        uint8_t const * random, std::size_t random_length
    );

    LocalKey const * lookup_key(
        std::uint32_t id
    );

    std::size_t remove_key(
        std::uint32_t id
    );
};


std::size_t pickle_length(
    Account const & value
);


std::uint8_t * pickle(
    std::uint8_t * pos,
    Account const & value
);


std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    Account & value
);


} // namespace axolotl

#endif /* AXOLOTL_ACCOUNT_HH_ */
