#include "olm/olm.hh"
#include "unittest.hh"

const char * test_cases[] = {
    "41776f",
    "7fff6f0101346d671201",
    "ee776f41496f674177804177778041776f6716670a677d6f670a67c2677d",
    "e9e9c9c1e9e9c9e9c9c1e9e9c9c1",
};


const char * session_data =
    "E0p44KO2y2pzp9FIjv0rud2wIvWDi2dx367kP4Fz/9JCMrH+aG369HGymkFtk0+PINTLB9lQRt"
    "ohea5d7G/UXQx3r5y4IWuyh1xaRnojEZQ9a5HRZSNtvmZ9NY1f1gutYa4UtcZcbvczN8b/5Bqg"
    "e16cPUH1v62JKLlhoAJwRkH1wU6fbyOudERg5gdXA971btR+Q2V8GKbVbO5fGKL5phmEPVXyMs"
    "rfjLdzQrgjOTxN8Pf6iuP+WFPvfnR9lDmNCFxJUVAdLIMnLuAdxf1TGcS+zzCzEE8btIZ99mHF"
    "dGvPXeH8qLeNZA";

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

void decrypt_case(int message_type, const char * test_case) {
    std::uint8_t session_memory[olm_session_size()];
    ::OlmSession * session = ::olm_session(session_memory);

    std::uint8_t pickled[strlen(session_data)];
    ::memcpy(pickled, session_data, sizeof(pickled));
    ::olm_unpickle_session(session, "", 0, pickled, sizeof(pickled));

    std::size_t message_length = strlen(test_case) / 2;
    std::uint8_t * message = (std::uint8_t *) ::malloc(message_length);
    decode_hex(test_case, message, message_length);

    size_t max_length = olm_decrypt_max_plaintext_length(
        session, message_type, message, message_length
    );

    if (max_length == std::size_t(-1)) {
        free(message);
        return;
    }

    uint8_t plaintext[max_length];
    decode_hex(test_case, message, message_length);
    olm_decrypt(
        session, message_type,
        message, message_length,
        plaintext, max_length
    );
    free(message);
}


int main() {
{
TestCase my_test("Olm decrypt test");

for (int i = 0; i < sizeof(test_cases)/ sizeof(const char *); ++i) {
    decrypt_case(0, test_cases[i]);
}

}
}
