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
#include <cstddef>
#include <cstdint>

namespace axolotl {

/** Clear the memory held in the buffer */
void unset(
    void volatile * buffer, std::size_t buffer_length
);

/** Clear the memory backing an object */
template<typename T>
void unset(T & value) {
    unset(reinterpret_cast<void volatile *>(&value), sizeof(T));
}

/** Check if two buffers are equal in constant time. */
bool is_equal(
    std::uint8_t const * buffer_a,
    std::uint8_t const * buffer_b,
    std::size_t length
);

} // namespace axolotl
