#include "axolotl/axolotl.hh"
#include "unittest.hh"

#include <cstddef>
#include <cstdint>
#include <cstring>

int main() {

{ /** Pickle account test */

TestCase test_case("Pickle account test");

std::uint8_t account_buffer[::axolotl_account_size()];
::AxolotlAccount *account = ::axolotl_account(account_buffer);
std::size_t random_length = ::axolotl_create_account_random_length(account);
std::uint8_t random[random_length];
std::memset(random, 4, random_length); /* http://xkcd.com/221/ */
::axolotl_create_account(account, random, random_length);
std::size_t pickle_length = ::axolotl_pickle_account_length(account);
std::uint8_t pickle1[pickle_length];
::axolotl_pickle_account(account, "secret_key", 10, pickle1, pickle_length);
std::uint8_t pickle2[pickle_length];
std::memcpy(pickle2, pickle1, pickle_length);

std::uint8_t account_buffer2[::axolotl_account_size()];
::AxolotlAccount *account2 = ::axolotl_account(account_buffer2);
::axolotl_unpickle_account(account2, "secret_key", 10, pickle2, pickle_length);
assert_equals(pickle_length, ::axolotl_pickle_account_length(account2));
::axolotl_pickle_account(account2, "secret_key", 10, pickle2, pickle_length);

assert_equals(pickle1, pickle2, pickle_length);

}

{ /** Loopback test */

TestCase test_case("Loopback test");

std::uint8_t a_account_buffer[::axolotl_account_size()];
::AxolotlAccount *a_account = ::axolotl_account(a_account_buffer);
std::uint8_t a_random[::axolotl_create_account_random_length(a_account)];
std::memset(a_random, 4, sizeof(a_random)); /* http://xkcd.com/221/ */
::axolotl_create_account(a_account, a_random, sizeof(a_random));

std::uint8_t b_account_buffer[::axolotl_account_size()];
::AxolotlAccount *b_account = ::axolotl_account(b_account_buffer);
std::uint8_t b_random[::axolotl_create_account_random_length(b_account)];
std::memset(b_random, 5, sizeof(b_random)); /* http://xkcd.com/221/ */
::axolotl_create_account(b_account, b_random, sizeof(b_random));

std::uint8_t b_id_keys[::axolotl_account_identity_keys_length(b_account)];
std::uint8_t b_ot_keys[::axolotl_account_one_time_keys_length(b_account)];
::axolotl_account_identity_keys(b_account, b_id_keys, sizeof(b_id_keys));
::axolotl_account_one_time_keys(b_account, b_ot_keys, sizeof(b_ot_keys));

std::uint8_t a_session_buffer[::axolotl_session_size()];
::AxolotlSession *a_session = ::axolotl_session(a_session_buffer);
std::uint8_t a_rand[::axolotl_create_outbound_session_random_length(a_session)];
std::memset(a_rand, 6, sizeof(a_rand)); /* http://xkcd.com/221/ */
assert_not_equals(std::size_t(-1), ::axolotl_create_outbound_session(
    a_session, a_account,
    b_id_keys + 14, 43,
    ::atol((char *)(b_ot_keys + 62)), b_ot_keys + 74, 43,
    a_rand, sizeof(a_rand)
));

std::uint8_t plaintext[] = "Hello, World";
std::uint8_t message_1[::axolotl_encrypt_message_length(a_session, 12)];
std::uint8_t a_message_random[::axolotl_encrypt_random_length(a_session)];
std::memset(a_message_random, 1, sizeof(a_message_random)); /* http://xkcd.com/221/ */
assert_equals(std::size_t(0), ::axolotl_encrypt_message_type(a_session));
assert_not_equals(std::size_t(-1), ::axolotl_encrypt(
    a_session,
    plaintext, 12,
    a_message_random, sizeof(a_message_random),
    message_1, sizeof(message_1)
));


std::uint8_t tmp_message_1[sizeof(message_1)];
std::memcpy(tmp_message_1, message_1, sizeof(message_1));
std::uint8_t b_session_buffer[::axolotl_account_size()];
::AxolotlSession *b_session = ::axolotl_session(b_session_buffer);
::axolotl_create_inbound_session(
    b_session, b_account, tmp_message_1, sizeof(message_1)
);

std::memcpy(tmp_message_1, message_1, sizeof(message_1));
std::uint8_t plaintext_1[::axolotl_decrypt_max_plaintext_length(
    b_session, 0, tmp_message_1, sizeof(message_1)
)];
std::memcpy(tmp_message_1, message_1, sizeof(message_1));
assert_equals(std::size_t(12), ::axolotl_decrypt(
    b_session, 0,
    tmp_message_1, sizeof(message_1),
    plaintext_1, sizeof(plaintext_1)
));

assert_equals(plaintext, plaintext_1, 12);

std::uint8_t message_2[::axolotl_encrypt_message_length(b_session, 12)];
std::uint8_t b_message_random[::axolotl_encrypt_random_length(b_session)];
std::memset(b_message_random, 2, sizeof(b_message_random)); /* http://xkcd.com/221/ */
assert_equals(std::size_t(1), ::axolotl_encrypt_message_type(b_session));
assert_not_equals(std::size_t(-1), ::axolotl_encrypt(
    b_session,
    plaintext, 12,
    b_message_random, sizeof(b_message_random),
    message_2, sizeof(message_2)
));

std::uint8_t tmp_message_2[sizeof(message_2)];
std::memcpy(tmp_message_2, message_2, sizeof(message_2));
std::uint8_t plaintext_2[::axolotl_decrypt_max_plaintext_length(
    a_session, 1, tmp_message_2, sizeof(message_2)
)];
std::memcpy(tmp_message_2, message_2, sizeof(message_2));
assert_equals(std::size_t(12), ::axolotl_decrypt(
    a_session, 1,
    tmp_message_2, sizeof(message_2),
    plaintext_2, sizeof(plaintext_2)
));

assert_equals(plaintext, plaintext_2, 12);

std::memcpy(tmp_message_2, message_2, sizeof(message_2));
assert_equals(std::size_t(-1), ::axolotl_decrypt(
    a_session, 1,
    tmp_message_2, sizeof(message_2),
    plaintext_2, sizeof(plaintext_2)
));


}

}
