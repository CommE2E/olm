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
#ifndef OLM_ACCOUNT_HH_
#define OLM_ACCOUNT_HH_

#include "olm/list.hh"
#include "olm/crypto.hh"
#include "olm/error.hh"

#include <cstdint>

namespace olm {


struct IdentityKeys {
    Ed25519KeyPair ed25519_key;
    Curve25519KeyPair curve25519_key;
};

struct OneTimeKey {
    std::uint32_t id;
    Curve25519KeyPair key;
};


static std::size_t const MAX_ONE_TIME_KEYS = 100;


struct Account {
    IdentityKeys identity_keys;
    List<OneTimeKey, MAX_ONE_TIME_KEYS> one_time_keys;
    ErrorCode last_error;

    /** Number of random bytes needed to create a new account */
    std::size_t new_account_random_length();

    /** Create a new account. Returns NOT_ENOUGH_RANDOM if the number of random
     * bytes is too small. */
    std::size_t new_account(
        uint8_t const * random, std::size_t random_length
    );

    /** Number of bytes needed to output the identity keys for this account */
    std::size_t get_identity_json_length(
        std::size_t user_id_length,
        std::size_t device_id_length,
        std::uint64_t valid_after_ts,
        std::uint64_t valid_until_ts
    );

    /** Output the identity keys for this account as JSON in the following
     * format.
     *
     *  14 "{\"algorithms\":"
     *  30 "[\"m.olm.curve25519-aes-sha256\""
     *  15 "],\"device_id\":\""
     *   ? <device identifier>
     *  22 "\",\"keys\":{\"curve25519:"
     *   4 <base64 characters>
     *   3 "\":\""
     *  43 <base64 characters>
     *  11 "\",\"ed25519:"
     *   4 <base64 characters>
     *   3 "\":\""
     *  43 <base64 characters>
     *  14 "\"},\"user_id\":\""
     *   ? <user identifier>
     *  19 "\",\"valid_after_ts\":"
     *   ? <digits>
     *  18 ",\"valid_until_ts\":"
     *   ? <digits>
     *  16 ",\"signatures\":{\""
     *   ? <user identifier>
     *   1 "/"
     *   ? <device identifier>
     *  12 "\":{\"ed25519:"
     *   4 <base64 characters>
     *   3 "\":\""
     *  86 <base64 characters>
     *   4 "\"}}}"
     */
    std::size_t get_identity_json(
        std::uint8_t const * user_id, std::size_t user_id_length,
        std::uint8_t const * device_id, std::size_t device_id_length,
        std::uint64_t valid_after_ts,
        std::uint64_t valid_until_ts,
        std::uint8_t * identity_json, std::size_t identity_json_length
    );

    OneTimeKey const * lookup_key(
        Curve25519PublicKey const & public_key
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


} // namespace olm

#endif /* OLM_ACCOUNT_HH_ */
