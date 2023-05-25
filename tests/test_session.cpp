#include "olm/session.hh"
#include "olm/pickle_encoding.h"

#include "testing.hh"

/* decode into a buffer, which is returned */
const std::uint8_t *decode_hex(
    const char * input
) {
    static std::uint8_t buf[256];
    std::uint8_t *p = buf;
    while (*input != '\0') {
        char high = *(input++);
        char low = *(input++);
        if (high >= 'a') high -= 'a' - ('9' + 1);
        if (low >= 'a') low -= 'a' - ('9' + 1);
        uint8_t value = ((high - '0') << 4) | (low - '0');
        *p++ = value;
    }
    return buf;
}

void check_session(const olm::Session &session) {
    CHECK_EQ_SIZE(
        decode_hex("d36e599136bc6a4197f7096e874ea8c462fae66b79b33c6a875a3dffe65b419c"),
        session.ratchet.root_key, 32
    );

    CHECK_EQ(
        std::size_t(1),
        session.ratchet.sender_chain.size()
    );

    CHECK_EQ_SIZE(
        decode_hex("3818e81e3404680419631167f5e8e34a38880f0811e28d73d21850ae0870421f"),
        session.ratchet.sender_chain[0].ratchet_key.public_key.public_key, 32
    );

    CHECK_EQ_SIZE(
        decode_hex("4104040404040404040404040404040404040404040404040404040404040404"),
        session.ratchet.sender_chain[0].ratchet_key.private_key.private_key, 32
    );

    CHECK_EQ(
        std::uint32_t(0),
        session.ratchet.sender_chain[0].chain_key.index
    );

    CHECK_EQ(
        std::size_t(0),
        session.ratchet.receiver_chains.size()
    );

    CHECK_EQ(
        std::size_t(0),
        session.ratchet.skipped_message_keys.size()
    );

    CHECK_EQ(OLM_SUCCESS, session.last_error);
    CHECK_EQ(false, session.received_message);

    CHECK_EQ_SIZE(
        decode_hex("de2e9d179b4ae14301f914a01d1d60e878bd8522a9a74cfc6a552b6329fd754c"),
        session.alice_identity_key.public_key, 32
    );

    CHECK_EQ_SIZE(
        decode_hex("d60e4d2eb6f8dc8535d09688ae3471fa51d81df8c7c8c55704baa751e24ddd12"),
        session.alice_base_key.public_key, 32
    );

    CHECK_EQ_SIZE(
        decode_hex("a21b0cfdb15a5035ab93279086d7a8d5410e326e71641ef2f9c0580581089326"),
        session.bob_one_time_key.public_key, 32
    );
}

TEST_CASE("V1 session pickle") {

    const uint8_t *PICKLE_KEY=(uint8_t *)"secret_key";
    uint8_t pickled[] =
        "jfeWFTiR6UrMw1bfBAiq8boj5VyCU8mv8T7zsn3FvtLJKET1OUg3B/RdSza+TtgfNBo7sEkQh"
        "sBjr4IkWiL6eCxxqOksuJfsbtpDjs6wBEfi3UCNa9gyKQyrL9gQ80TqTjQoakkAIkJQxPBGBX"
        "kgxrPoItfykTNd+sWK0BBqyIhLCt55yzoEjoOUfhAEteA/oZE/Vfs783NmnQwee3uwUzyfMUm"
        "kewQkSGjdXtfULdWcne6fh8FXpe7s9ZILzDPrWYiozuRt2g2ANPxf6si9YsoI3BGs56hrn/KE"
        "I27SyFPh2DOq5UY+M7B/dPHvufvrBryDGJ0J0G6VH4MFD3sDr92Skm/UY5OV/Yclx+T/DW4ZD"
        "wjEMK+DV7DytCKBTXEb2kYArnb4a50";
    size_t pickle_len = _olm_enc_input(
        PICKLE_KEY, strlen((char *)PICKLE_KEY),
        pickled, strlen((char *)pickled), NULL
    );

    olm::Session session;
    const uint8_t *unpickle_res = olm::unpickle(pickled, pickled+sizeof(pickled), session);
    CHECK_EQ(
        pickle_len, (size_t)(unpickle_res - pickled)
    );

    check_session(session);

#if 0
    size_t rawlen = olm::pickle_length(session);
    uint8_t *r1 = _olm_enc_output_pos(pickled, rawlen);
    olm::pickle(r1, session);
    _olm_enc_output(
        PICKLE_KEY, strlen((char *)PICKLE_KEY),
        pickled, rawlen);
    printf("%s\n", pickled);
#endif
}

TEST_CASE("V2 session pickle") {

    const uint8_t *PICKLE_KEY=(uint8_t *)"secret_key";
    uint8_t pickled[] =
        "jfeWFTiR6UrMw1bfBAiq8boj5VyCU8mv8T7zsn3FvtLJKET1OUg3B/RdSza+TtgfNBo7sEkQh"
        "sBjr4IkWiL6eCxxqOksuJfsbtpDjs6wBEfi3UCNa9gyKQyrL9gQ80TqTjQoakkAIkJQxPBGBX"
        "kgxrPoItfykTNd+sWK0BBqyIhLCt55yzoEjoOUfhAEteA/oZE/Vfs783NmnQwee3uwUzyfMUm"
        "kewQkSGjdXtfULdWcne6fh8FXpe7s9ZILzDPrWYiozuRt2g2ANPxf6si9YsoI3BGs56hrn/KE"
        "I27SyFPh2DOq5UY+M7B/dPHvufvrBryDGJ0J0G6VH4MFD3sDr92Skm/UY5OV/Yclx+T/DW4ZD"
        "wjEMK+DV7DytCKBTXEb2kYArnb4a50";

    size_t pickle_len = _olm_enc_input(
        PICKLE_KEY, strlen((char *)PICKLE_KEY),
        pickled, strlen((char *)pickled), NULL
    );

    olm::Session session;
    const uint8_t *unpickle_res = olm::unpickle(pickled, pickled+sizeof(pickled), session);
    CHECK_EQ(
        pickle_len, (size_t)(unpickle_res - pickled)
    );

    check_session(session);
}
