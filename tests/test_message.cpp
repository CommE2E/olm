/* Copyright 2015-2016 OpenMarket Ltd
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
#include "olm/message.hh"
#include "unittest.hh"

int main() {

std::uint8_t message1[36] = "\x03\x10\x01\n\nratchetkey\"\nciphertexthmacsha2";
std::uint8_t message2[36] = "\x03\n\nratchetkey\x10\x01\"\nciphertexthmacsha2";
std::uint8_t ratchetkey[11] = "ratchetkey";
std::uint8_t ciphertext[11] = "ciphertext";
std::uint8_t hmacsha2[9] = "hmacsha2";

{ /* Message decode test */

TestCase test_case("Message decode test");

olm::MessageReader reader;
olm::decode_message(reader, message1, 35, 8);

assert_equals(std::uint8_t(3), reader.version);
assert_equals(true, reader.has_counter);
assert_equals(std::uint32_t(1), reader.counter);
assert_equals(std::size_t(10), reader.ratchet_key_length);
assert_equals(std::size_t(10), reader.ciphertext_length);

assert_equals(ratchetkey, reader.ratchet_key, 10);
assert_equals(ciphertext, reader.ciphertext, 10);


} /* Message decode test */

{ /* Message encode test */

TestCase test_case("Message encode test");

std::size_t length = olm::encode_message_length(1, 10, 10, 8);
assert_equals(std::size_t(35), length);

std::uint8_t output[35];

olm::MessageWriter writer;
olm::encode_message(writer, 3, 1, 10, 10, output);

std::memcpy(writer.ratchet_key, ratchetkey, 10);
std::memcpy(writer.ciphertext, ciphertext, 10);
std::memcpy(output + length - 8, hmacsha2, 8);

assert_equals(message2, output, 35);

} /* Message encode test */


{ /* group message encode test */

    TestCase test_case("Group message encode test");

    size_t length = _olm_encode_group_message_length(200, 10, 8, 64);
    size_t expected_length = 1 + (1+2) + (2+10) + 8 + 64;
    assert_equals(expected_length, length);

    uint8_t output[50];
    uint8_t *ciphertext_ptr;

    _olm_encode_group_message(
        3,
        200, // counter
        10,  // ciphertext length
        output,
        &ciphertext_ptr
    );

    uint8_t expected[] =
        "\x03"
        "\x08\xC8\x01"
        "\x12\x0A";

    assert_equals(expected, output, sizeof(expected)-1);
    assert_equals(output+sizeof(expected)-1, ciphertext_ptr);
} /* group message encode test */

{
    TestCase test_case("Group message decode test");

    struct _OlmDecodeGroupMessageResults results;
    std::uint8_t message[] =
        "\x03"
        "\x08\xC8\x01"
        "\x12\x0A" "ciphertext"
        "hmacsha2"
        "ed25519signature";

    _olm_decode_group_message(message, sizeof(message)-1, 8, 16, &results);
    assert_equals(std::uint8_t(3), results.version);
    assert_equals(1, results.has_message_index);
    assert_equals(std::uint32_t(200), results.message_index);
    assert_equals(std::size_t(10), results.ciphertext_length);
    assert_equals(ciphertext, results.ciphertext, 10);
} /* group message decode test */
}
