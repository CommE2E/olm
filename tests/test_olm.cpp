#include "olm/olm.hh"
#include "unittest.hh"

#include <cstddef>
#include <cstdint>
#include <cstring>

struct MockRandom {
    MockRandom(std::uint8_t tag, std::uint8_t offset = 0)
        : tag(tag), current(offset) {}
    void operator()(
        std::uint8_t * bytes, std::size_t length
    ) {
        while (length > 32) {
            bytes[0] = tag;
            std::memset(bytes + 1, current, 31);
            length -= 32;
            bytes += 32;
            current += 1;
        }
        if (length) {
            bytes[0] = tag;
            std::memset(bytes + 1, current, length - 1);
            current += 1;
        }
    }
    std::uint8_t tag;
    std::uint8_t current;
};

int main() {

{ /** Pickle account test */

TestCase test_case("Pickle account test");
MockRandom mock_random('P');


std::uint8_t account_buffer[::olm_account_size()];
::OlmAccount *account = ::olm_account(account_buffer);
std::uint8_t random[::olm_create_account_random_length(account)];
mock_random(random, sizeof(random));
::olm_create_account(account, random, sizeof(random));
std::size_t pickle_length = ::olm_pickle_account_length(account);
std::uint8_t pickle1[pickle_length];
::olm_pickle_account(account, "secret_key", 10, pickle1, pickle_length);
std::uint8_t pickle2[pickle_length];
std::memcpy(pickle2, pickle1, pickle_length);

std::uint8_t account_buffer2[::olm_account_size()];
::OlmAccount *account2 = ::olm_account(account_buffer2);
::olm_unpickle_account(account2, "secret_key", 10, pickle2, pickle_length);
assert_equals(pickle_length, ::olm_pickle_account_length(account2));
::olm_pickle_account(account2, "secret_key", 10, pickle2, pickle_length);

assert_equals(pickle1, pickle2, pickle_length);

}

{ /** Loopback test */

TestCase test_case("Loopback test");
MockRandom mock_random_a('A', 0x00);
MockRandom mock_random_b('B', 0x80);

std::uint8_t a_account_buffer[::olm_account_size()];
::OlmAccount *a_account = ::olm_account(a_account_buffer);
std::uint8_t a_random[::olm_create_account_random_length(a_account)];
mock_random_a(a_random, sizeof(a_random));
::olm_create_account(a_account, a_random, sizeof(a_random));

std::uint8_t b_account_buffer[::olm_account_size()];
::OlmAccount *b_account = ::olm_account(b_account_buffer);
std::uint8_t b_random[::olm_create_account_random_length(b_account)];
mock_random_b(b_random, sizeof(b_random));
::olm_create_account(b_account, b_random, sizeof(b_random));

std::uint8_t b_id_keys[::olm_account_identity_keys_length(b_account)];
std::uint8_t b_ot_keys[::olm_account_one_time_keys_length(b_account)];
::olm_account_identity_keys(b_account, b_id_keys, sizeof(b_id_keys));
::olm_account_one_time_keys(b_account, b_ot_keys, sizeof(b_ot_keys));

std::uint8_t a_session_buffer[::olm_session_size()];
::OlmSession *a_session = ::olm_session(a_session_buffer);
std::uint8_t a_rand[::olm_create_outbound_session_random_length(a_session)];
mock_random_a(a_rand, sizeof(a_rand));
assert_not_equals(std::size_t(-1), ::olm_create_outbound_session(
    a_session, a_account,
    b_id_keys + 14, 43,
    ::atol((char *)(b_ot_keys + 62)), b_ot_keys + 74, 43,
    a_rand, sizeof(a_rand)
));

std::uint8_t plaintext[] = "Hello, World";
std::uint8_t message_1[::olm_encrypt_message_length(a_session, 12)];
std::uint8_t a_message_random[::olm_encrypt_random_length(a_session)];
mock_random_a(a_message_random, sizeof(a_message_random));
assert_equals(std::size_t(0), ::olm_encrypt_message_type(a_session));
assert_not_equals(std::size_t(-1), ::olm_encrypt(
    a_session,
    plaintext, 12,
    a_message_random, sizeof(a_message_random),
    message_1, sizeof(message_1)
));


std::uint8_t tmp_message_1[sizeof(message_1)];
std::memcpy(tmp_message_1, message_1, sizeof(message_1));
std::uint8_t b_session_buffer[::olm_account_size()];
::OlmSession *b_session = ::olm_session(b_session_buffer);
::olm_create_inbound_session(
    b_session, b_account, tmp_message_1, sizeof(message_1)
);

std::memcpy(tmp_message_1, message_1, sizeof(message_1));
std::uint8_t plaintext_1[::olm_decrypt_max_plaintext_length(
    b_session, 0, tmp_message_1, sizeof(message_1)
)];
std::memcpy(tmp_message_1, message_1, sizeof(message_1));
assert_equals(std::size_t(12), ::olm_decrypt(
    b_session, 0,
    tmp_message_1, sizeof(message_1),
    plaintext_1, sizeof(plaintext_1)
));

assert_equals(plaintext, plaintext_1, 12);

std::uint8_t message_2[::olm_encrypt_message_length(b_session, 12)];
std::uint8_t b_message_random[::olm_encrypt_random_length(b_session)];
mock_random_b(b_message_random, sizeof(b_message_random));
assert_equals(std::size_t(1), ::olm_encrypt_message_type(b_session));
assert_not_equals(std::size_t(-1), ::olm_encrypt(
    b_session,
    plaintext, 12,
    b_message_random, sizeof(b_message_random),
    message_2, sizeof(message_2)
));

std::uint8_t tmp_message_2[sizeof(message_2)];
std::memcpy(tmp_message_2, message_2, sizeof(message_2));
std::uint8_t plaintext_2[::olm_decrypt_max_plaintext_length(
    a_session, 1, tmp_message_2, sizeof(message_2)
)];
std::memcpy(tmp_message_2, message_2, sizeof(message_2));
assert_equals(std::size_t(12), ::olm_decrypt(
    a_session, 1,
    tmp_message_2, sizeof(message_2),
    plaintext_2, sizeof(plaintext_2)
));

assert_equals(plaintext, plaintext_2, 12);

std::memcpy(tmp_message_2, message_2, sizeof(message_2));
assert_equals(std::size_t(-1), ::olm_decrypt(
    a_session, 1,
    tmp_message_2, sizeof(message_2),
    plaintext_2, sizeof(plaintext_2)
));

}

{ /** More messages test */

TestCase test_case("More messages test");
MockRandom mock_random_a('A', 0x00);
MockRandom mock_random_b('B', 0x80);

std::uint8_t a_account_buffer[::olm_account_size()];
::OlmAccount *a_account = ::olm_account(a_account_buffer);
std::uint8_t a_random[::olm_create_account_random_length(a_account)];
mock_random_a(a_random, sizeof(a_random));
::olm_create_account(a_account, a_random, sizeof(a_random));

std::uint8_t b_account_buffer[::olm_account_size()];
::OlmAccount *b_account = ::olm_account(b_account_buffer);
std::uint8_t b_random[::olm_create_account_random_length(b_account)];
mock_random_b(b_random, sizeof(b_random));
::olm_create_account(b_account, b_random, sizeof(b_random));

std::uint8_t b_id_keys[::olm_account_identity_keys_length(b_account)];
std::uint8_t b_ot_keys[::olm_account_one_time_keys_length(b_account)];
::olm_account_identity_keys(b_account, b_id_keys, sizeof(b_id_keys));
::olm_account_one_time_keys(b_account, b_ot_keys, sizeof(b_ot_keys));

std::uint8_t a_session_buffer[::olm_session_size()];
::OlmSession *a_session = ::olm_session(a_session_buffer);
std::uint8_t a_rand[::olm_create_outbound_session_random_length(a_session)];
mock_random_a(a_rand, sizeof(a_rand));
assert_not_equals(std::size_t(-1), ::olm_create_outbound_session(
    a_session, a_account,
    b_id_keys + 14, 43,
    ::atol((char *)(b_ot_keys + 62)), b_ot_keys + 74, 43,
    a_rand, sizeof(a_rand)
));

std::uint8_t plaintext[] = "Hello, World";
std::uint8_t message_1[::olm_encrypt_message_length(a_session, 12)];
std::uint8_t a_message_random[::olm_encrypt_random_length(a_session)];
mock_random_a(a_message_random, sizeof(a_message_random));
assert_equals(std::size_t(0), ::olm_encrypt_message_type(a_session));
assert_not_equals(std::size_t(-1), ::olm_encrypt(
    a_session,
    plaintext, 12,
    a_message_random, sizeof(a_message_random),
    message_1, sizeof(message_1)
));

std::uint8_t tmp_message_1[sizeof(message_1)];
std::memcpy(tmp_message_1, message_1, sizeof(message_1));
std::uint8_t b_session_buffer[::olm_account_size()];
::OlmSession *b_session = ::olm_session(b_session_buffer);
::olm_create_inbound_session(
    b_session, b_account, tmp_message_1, sizeof(message_1)
);

std::memcpy(tmp_message_1, message_1, sizeof(message_1));
std::uint8_t plaintext_1[::olm_decrypt_max_plaintext_length(
    b_session, 0, tmp_message_1, sizeof(message_1)
)];
std::memcpy(tmp_message_1, message_1, sizeof(message_1));
assert_equals(std::size_t(12), ::olm_decrypt(
    b_session, 0,
    tmp_message_1, sizeof(message_1),
    plaintext_1, sizeof(plaintext_1)
));

for (unsigned i = 0; i < 8; ++i) {
    {
    std::uint8_t msg_a[::olm_encrypt_message_length(a_session, 12)];
    std::uint8_t rnd_a[::olm_encrypt_random_length(a_session)];
    mock_random_a(rnd_a, sizeof(rnd_a));
    std::size_t type_a = ::olm_encrypt_message_type(a_session);
    assert_not_equals(std::size_t(-1), ::olm_encrypt(
        a_session, plaintext, 12, rnd_a, sizeof(rnd_a), msg_a, sizeof(msg_a)
    ));

    std::uint8_t tmp_a[sizeof(msg_a)];
    std::memcpy(tmp_a, msg_a, sizeof(msg_a));
    std::uint8_t out_a[::olm_decrypt_max_plaintext_length(
        b_session, type_a, tmp_a, sizeof(tmp_a)
    )];
    std::memcpy(tmp_a, msg_a, sizeof(msg_a));
    assert_equals(std::size_t(12), ::olm_decrypt(
        b_session, type_a, msg_a, sizeof(msg_a), out_a, sizeof(out_a)
    ));
    }
    {
    std::uint8_t msg_b[::olm_encrypt_message_length(b_session, 12)];
    std::uint8_t rnd_b[::olm_encrypt_random_length(b_session)];
    mock_random_b(rnd_b, sizeof(rnd_b));
    std::size_t type_b = ::olm_encrypt_message_type(b_session);
    assert_not_equals(std::size_t(-1), ::olm_encrypt(
        b_session, plaintext, 12, rnd_b, sizeof(rnd_b), msg_b, sizeof(msg_b)
    ));

    std::uint8_t tmp_b[sizeof(msg_b)];
    std::memcpy(tmp_b, msg_b, sizeof(msg_b));
    std::uint8_t out_b[::olm_decrypt_max_plaintext_length(
        a_session, type_b, tmp_b, sizeof(tmp_b)
    )];
    std::memcpy(tmp_b, msg_b, sizeof(msg_b));
    assert_equals(std::size_t(12), ::olm_decrypt(
        a_session, type_b, msg_b, sizeof(msg_b), out_b, sizeof(out_b)
    ));
    }
}
}

}
