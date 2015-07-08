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
#ifndef OLM_BASE64_HH_
#define OLM_BASE64_HH_

#include <cstddef>
#include <cstdint>

namespace olm {


static std::size_t encode_base64_length(
    std::size_t input_length
) {
    return 4 * ((input_length + 2) / 3) + (input_length + 2) % 3 - 2;
}


std::uint8_t * encode_base64(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


std::size_t decode_base64_length(
    std::size_t input_length
);


std::uint8_t const * decode_base64(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


} // namespace olm


#endif /* OLM_BASE64_HH_ */
