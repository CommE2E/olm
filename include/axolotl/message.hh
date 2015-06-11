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

/**
 * The length of the buffer needed to hold a message.
 */
std::size_t encode_message_length(
    std::uint32_t counter,
    std::size_t ratchet_key_length,
    std::size_t ciphertext_length,
    std::size_t mac_length
);


struct MessageWriter {
    std::uint8_t * ratchet_key;
    std::uint8_t * ciphertext;
};


struct MessageReader {
    std::uint8_t version;
    std::uint32_t counter;
    std::uint8_t const * input; std::size_t input_length;
    std::uint8_t const * ratchet_key; std::size_t ratchet_key_length;
    std::uint8_t const * ciphertext; std::size_t ciphertext_length;
};


/**
 * Writes the message headers into the output buffer.
 * Returns a writer struct populated with pointers into the output buffer.
 */

void encode_message(
    MessageWriter & writer,
    std::uint8_t version,
    std::uint32_t counter,
    std::size_t ratchet_key_length,
    std::size_t ciphertext_length,
    std::uint8_t * output
);


/**
 * Reads the message headers from the input buffer.
 * Populates the reader struct with pointers into the input buffer.
 * On failure returns std::size_t(-1).
 */
std::size_t decode_message(
    MessageReader & reader,
    std::uint8_t const * input, std::size_t input_length,
    std::size_t mac_length
);


} // namespace axolotl
