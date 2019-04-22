#include "olm/olm.h"
#include "unittest.hh"

#include <vector>

int main() {
{
TestCase("Olm sha256 test");


std::vector<std::uint8_t> utility_buffer(::olm_utility_size());
::OlmUtility * utility = ::olm_utility(utility_buffer.data());

assert_equals(std::size_t(43), ::olm_sha256_length(utility));
std::uint8_t output[43];
::olm_sha256(utility, "Hello, World", 12, output, 43);

std::uint8_t expected_output[] = "A2daxT/5zRU1zMffzfosRYxSGDcfQY3BNvLRmsH76KU";
assert_equals(output, expected_output, 43);

}
}
