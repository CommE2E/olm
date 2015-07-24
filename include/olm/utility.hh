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

#ifndef UTILITY_HH_
#define UTILITY_HH_

#include "olm/error.hh"

#include <cstddef>
#include <cstdint>

namespace olm {

class Ed25519PublicKey;

struct Utility {

    Utility();

    ErrorCode last_error;

    std::size_t sha256_length();

    std::size_t sha256(
        std::uint8_t const * input, std::size_t input_length,
        std::uint8_t * output, std::size_t output_length
    );

    std::size_t ed25519_verify(
        Ed25519PublicKey const & key,
        std::uint8_t const * message, std::size_t message_length,
        std::uint8_t const * signature, std::size_t signature_length
    );

};


} // namespace olm

#endif /* UTILITY_HH_ */
