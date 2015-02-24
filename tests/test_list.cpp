#include "axolotl/list.hh"
#include "unittest.hh"

int main() {

{ /** List insert test **/

TestCase test_case("List insert");

axolotl::List<int, 4> test_list;

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

{ /** List erase test **/
TestCase test_case("List erase");

axolotl::List<int, 4> test_list;
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
