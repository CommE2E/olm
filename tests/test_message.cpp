#include "axolotl/message.hh"
#include "unittest.hh"

int main() {

std::uint8_t message1[36] = "\x03\n\nratchetkey\x10\x01\"\nciphertexthmacsha2";
std::uint8_t message2[36] = "\x03\x10\x01\n\nratchetkey\"\nciphertexthmacsha2";
std::uint8_t ratchetkey[11] = "ratchetkey";
std::uint8_t ciphertext[11] = "ciphertext";
std::uint8_t hmacsha2[9] = "hmacsha2";

{ /* Message decode test */

TestCase test_case("Message decode test");

axolotl::MessageReader reader(axolotl::decode_message(message1, 35, 8));

assert_equals(std::size_t(27), reader.body_length);
assert_equals(std::uint8_t(3), reader.version);
assert_equals(std::uint32_t(1), reader.counter);
assert_equals(std::size_t(10), reader.ratchet_key_length);
assert_equals(std::size_t(10), reader.ciphertext_length);

assert_equals(ratchetkey, reader.ratchet_key, 10);
assert_equals(ciphertext, reader.ciphertext, 10);
assert_equals(hmacsha2, reader.mac, 8);


} /* Message decode test */

{ /* Message encode test */

TestCase test_case("Message encode test");

std::size_t length = axolotl::encode_message_length(1, 10, 10, 8);
assert_equals(std::size_t(35), length);

std::uint8_t output[length];

axolotl::MessageWriter writer(axolotl::encode_message(3, 1, 10, 10, output));
assert_equals(std::size_t(27), writer.body_length);

std::memcpy(writer.ratchet_key, ratchetkey, 10);
std::memcpy(writer.ciphertext, ciphertext, 10);
std::memcpy(writer.mac, hmacsha2, 8);

assert_equals(message2, output, 35);

} /* Message encode test */

}
