#include "olm/base64.hh"
#include "olm/base64.h"
#include "unittest.hh"

int main() {

{ /* Base64 encode test */
TestCase test_case("Base64 C++ binding encode test");

std::uint8_t input[] = "Hello World";
std::uint8_t expected_output[] = "SGVsbG8gV29ybGQ";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = olm::encode_base64_length(input_length);
assert_equals(std::size_t(15), output_length);

std::uint8_t output[15];
olm::encode_base64(input, input_length, output);
assert_equals(expected_output, output, output_length);
}

{
TestCase test_case("Base64 C binding encode test");

std::uint8_t input[] = "Hello World";
std::uint8_t expected_output[] = "SGVsbG8gV29ybGQ";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = ::_olm_encode_base64_length(input_length);
assert_equals(std::size_t(15), output_length);

std::uint8_t output[15];
output_length = ::_olm_encode_base64(input, input_length, output);
assert_equals(std::size_t(15), output_length);
assert_equals(expected_output, output, output_length);
}

{ /* Base64 decode test */
TestCase test_case("Base64 C++ binding decode test");

std::uint8_t input[] = "SGVsbG8gV29ybGQ";
std::uint8_t expected_output[] = "Hello World";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = olm::decode_base64_length(input_length);
assert_equals(std::size_t(11), output_length);

std::uint8_t output[11];
olm::decode_base64(input, input_length, output);
assert_equals(expected_output, output, output_length);
}

{
TestCase test_case("Base64 C binding decode test");

std::uint8_t input[] = "SGVsbG8gV29ybGQ";
std::uint8_t expected_output[] = "Hello World";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = ::_olm_decode_base64_length(input_length);
assert_equals(std::size_t(11), output_length);

std::uint8_t output[11];
output_length = ::_olm_decode_base64(input, input_length, output);
assert_equals(std::size_t(11), output_length);
assert_equals(expected_output, output, output_length);
}

{
TestCase test_case("Decoding base64 of invalid length fails with -1");
#include <iostream>
std::uint8_t input[] = "SGVsbG8gV29ybGQab";
std::size_t input_length = sizeof(input) - 1;

/* We use a longer but valid input length here so that we don't get back -1.
 * Nothing will be written to the output buffer anyway because the input is
 * invalid. */
std::size_t buf_length = olm::decode_base64_length(input_length + 1);
std::uint8_t output[buf_length];
std::uint8_t expected_output[buf_length];
memset(output, 0, buf_length);
memset(expected_output, 0, buf_length);

std::size_t output_length = ::_olm_decode_base64(input, input_length, output);
assert_equals(std::size_t(-1), output_length);
assert_equals(0, memcmp(output, expected_output, buf_length));
}

}
