#include <cstddef>
#include <cstdint>


namespace axolotl {

/**
 * The length of the buffer needed to hold a message.
 */
std::size_t encode_message_length(
    std::uint32_t counter,
    std::size_t ratchet_key_length,
    std::size_t ciphertext_length,
    std::size_t mac_length
);


struct MessageWriter {
    std::size_t body_length;
    std::uint8_t * ratchet_key;
    std::uint8_t * ciphertext;
    std::uint8_t * mac;
};


struct MessageReader {
    std::size_t body_length;
    std::uint8_t version;
    std::uint32_t counter;
    std::size_t ratchet_key_length;
    std::size_t ciphertext_length;
    std::size_t mac_length;
    std::uint8_t const * ratchet_key;
    std::uint8_t const * ciphertext;
    std::uint8_t const * mac;
};


/**
 * Writes the message headers into the output buffer.
 * Returns a writer struct populated with pointers into the output buffer.
 */
MessageWriter encode_message(
    std::uint8_t version,
    std::uint32_t counter,
    std::size_t ratchet_key_length,
    std::size_t ciphertext_length,
    std::uint8_t * output
);


/**
 * Reads the message headers from the input buffer.
 * Returns a reader struct populated with pointers into the input buffer.
 * On failure the returned body_length will be 0.
 */
MessageReader decode_message(
    std::uint8_t const * input, std::size_t input_length,
    std::size_t mac_length
);


} // namespace axolotl
