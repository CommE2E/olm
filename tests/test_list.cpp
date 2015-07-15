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
#include "olm/list.hh"
#include "unittest.hh"

int main() {

{ /** List insert test **/

TestCase test_case("List insert");

olm::List<int, 4> test_list;

assert_equals(std::size_t(0), test_list.size());

for (int i = 0; i < 4; ++i) {
    test_list.insert(test_list.end(), i);
}

assert_equals(std::size_t(4), test_list.size());

int i = 0;
for (auto item : test_list) {
    assert_equals(i++, item);
}

assert_equals(4, i);

test_list.insert(test_list.end(), 4);

assert_equals(4, test_list[3]);

} /** List insert test **/

{ /** List insert beginning test **/

TestCase test_case("List insert beginning");

olm::List<int, 4> test_list;

assert_equals(std::size_t(0), test_list.size());

for (int i = 0; i < 4; ++i) {
    test_list.insert(test_list.begin(), i);
}

assert_equals(std::size_t(4), test_list.size());

int i = 4;
for (auto item : test_list) {
    assert_equals(--i, item);
}

} /** List insert test **/


{ /** List erase test **/
TestCase test_case("List erase");

olm::List<int, 4> test_list;
assert_equals(std::size_t(0), test_list.size());

for (int i = 0; i < 4; ++i) {
    test_list.insert(test_list.end(), i);
}
assert_equals(std::size_t(4), test_list.size());

test_list.erase(test_list.begin());
assert_equals(std::size_t(3), test_list.size());

int i = 0;
for (auto item : test_list) {
    assert_equals(i + 1, item);
    ++i;
}
assert_equals(3, i);

}

}
