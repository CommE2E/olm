
#include "axolotl/base64.hh"
#include "unittest.hh"

int main() {

{ /* Base64 encode test */
TestCase test_case("Base64 encode test");

std::uint8_t input[] = "Hello World";
std::uint8_t expected_output[] = "SGVsbG8gV29ybGQ";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = axolotl::encode_base64_length(input_length);
assert_equals(std::size_t(15), output_length);

std::uint8_t output[output_length];
axolotl::encode_base64(input, input_length, output);
assert_equals(expected_output, output, output_length);
}

{ /* Base64 decode test */
TestCase test_case("Base64 decode test");

std::uint8_t input[] = "SGVsbG8gV29ybGQ";
std::uint8_t expected_output[] = "Hello World";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = axolotl::decode_base64_length(input_length);
assert_equals(std::size_t(11), output_length);

std::uint8_t output[output_length];
axolotl::decode_base64(input, input_length, output);
assert_equals(expected_output, output, output_length);
}


}
