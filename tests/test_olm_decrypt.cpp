#include "olm/olm.h"
#include "testing.hh"

#include <vector>

struct test_case {
    const char *msghex;
    const char *expected_error;
};

const test_case test_cases[] = {
    { "41776f", "BAD_MESSAGE_FORMAT" },
    { "7fff6f0101346d671201", "BAD_MESSAGE_FORMAT" },
    { "ee776f41496f674177804177778041776f6716670a677d6f670a67c2677d", "BAD_MESSAGE_FORMAT" },
    { "e9e9c9c1e9e9c9e9c9c1e9e9c9c1", "BAD_MESSAGE_FORMAT" },
};


const char * session_data =
    "BEHrQRLy+I+z1ovctspbZ16PLm4RYVriIfN0OV2k8TKYqAzk6QPkasAZe19sGn7SCVfU+x90S9G5WmlTF"
    "SoOo6TWuzwD9YkK9OjEu33i24AjMmXKESi8kfE+84UguuBJSEqEeJqV3yQHFLEWj5PUqgGBnD/pWMSpk3"
    "7dmLlc2HSIr+QIuzl7fpyA09yyF9/IkSFbxAyqg94Ce7wQSw4BbKeXvDDK9crLShakUR7VC3nZCSzZs8s"
    "C8NeGOHV9nhl6oJWBMnKoCFw";

void decode_hex(
    const char * input,
    std::uint8_t * output, std::size_t output_length
) {
    std::uint8_t * end = output + output_length;
    while (output != end) {
        char high = *(input++);
        char low = *(input++);
        if (high >= 'a') high -= 'a' - ('9' + 1);
        if (low >= 'a') low -= 'a' - ('9' + 1);
        uint8_t value = ((high - '0') << 4) | (low - '0');
        *(output++) = value;
    }
}

void decrypt_case(int message_type, const test_case * test_case) {
    std::vector<std::uint8_t> session_memory(olm_session_size());
    ::OlmSession * session = ::olm_session(session_memory.data());

    std::vector<std::uint8_t> pickled(strlen(session_data));
    ::memcpy(pickled.data(), session_data, pickled.size());
    CHECK_NE(
        ::olm_error(),
        ::olm_unpickle_session(session, "", 0, pickled.data(), pickled.size())
    );

    std::size_t message_length = strlen(test_case->msghex) / 2;
    std::uint8_t * message = (std::uint8_t *) ::malloc(message_length);
    decode_hex(test_case->msghex, message, message_length);

    size_t max_length = olm_decrypt_max_plaintext_length(
        session, message_type, message, message_length
    );

    if (test_case->expected_error) {
        CHECK_EQ(::olm_error(), max_length);
        CHECK_EQ(
            std::string(test_case->expected_error),
            std::string(::olm_session_last_error(session))
        );
        free(message);
        return;
    }

    CHECK_NE(::olm_error(), max_length);

    std::vector<uint8_t> plaintext(max_length);
    decode_hex(test_case->msghex, message, message_length);
    olm_decrypt(
        session, message_type,
        message, message_length,
        plaintext.data(), max_length
    );
    free(message);
}


TEST_CASE("Olm decrypt test") {

for (const auto& test_case : test_cases) {
    CAPTURE(test_case.msghex);
    decrypt_case(0, &test_case);
}

}
