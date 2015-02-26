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
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cstdlib>


std::ostream & print_hex(
    std::ostream & os,
    std::uint8_t const * data,
    std::size_t length
) {
    for (std::size_t i = 0; i < length; i++) {
        os << std::setw(2) << std::setfill('0') << std::right
            << std::hex << (int) data[i];
    }
    return os;
}


char const * TEST_CASE;


template<typename T>
void assert_equals(
    T const & expected,
    T const & actual
) {
    if (expected != actual) {
        std::cout << "FAILED: " << TEST_CASE << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Actual:   " << actual << std::endl;
        std::exit(1);
    }
}


void assert_equals(
    std::uint8_t const * expected,
    std::uint8_t const * actual,
    std::size_t length
) {
    if (std::memcmp(expected, actual, length)) {
        std::cout << "FAILED: " << TEST_CASE << std::endl;
        print_hex(std::cout << "Expected: ", expected, length) << std::endl;
        print_hex(std::cout << "Actual:   ", actual, length) << std::endl;
        std::exit(1);
    }
}


class TestCase  {
public:
    TestCase(const char *name) { TEST_CASE = name; }
    ~TestCase() {  std::cout << "PASSED: " << TEST_CASE << std::endl; }
};
