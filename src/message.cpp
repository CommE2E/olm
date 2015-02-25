#include "axolotl/message.hh"

namespace {

template<typename T>
std::size_t varint_length(
    T value
) {
    std::size_t result = 1;
    while (value > 128U) {
        ++result;
        value >>= 7;
    }
    return result;
}


template<typename T>
std::uint8_t * varint_encode(
    std::uint8_t * output,
    T value
) {
    while (value > 128U) {
        *(output++) = (0x7F & value) | 0x80;
    }
    (*output++) = value;
    return output;
}


template<typename T>
T varint_decode(
    std::uint8_t const * varint_start,
    std::uint8_t const * varint_end
) {
    T value = 0;
    do {
        value <<= 7;
        value |= 0x7F & *(--varint_end);
    } while (varint_end != varint_start);
    return value;
}


std::uint8_t const * varint_skip(
    std::uint8_t const * input,
    std::uint8_t const * input_end
) {
    while (input != input_end) {
        std::uint8_t tmp = *(input++);
        if ((tmp & 0x80) == 0) {
            return input;
        }
    }
    return input;
}


std::size_t varstring_length(
    std::size_t string_length
) {
    return varint_length(string_length) + string_length;
}

static std::size_t const VERSION_LENGTH = 1;
static std::uint8_t const RATCHET_KEY_TAG = 012;
static std::uint8_t const COUNTER_TAG = 020;
static std::uint8_t const CIPHERTEXT_TAG = 042;

} // namespace


std::size_t axolotl::encode_message_length(
    std::uint32_t counter,
    std::size_t ratchet_key_length,
    std::size_t ciphertext_length,
    std::size_t mac_length
) {
    std::size_t length = VERSION_LENGTH;
    length += 1 + varstring_length(ratchet_key_length);
    length += 1 + varint_length(counter);
    length += 1 + varstring_length(ciphertext_length);
    return length + mac_length;
}


axolotl::MessageWriter axolotl::encode_message(
    std::uint8_t version,
    std::uint32_t counter,
    std::size_t ratchet_key_length,
    std::size_t ciphertext_length,
    std::uint8_t * output
) {
    axolotl::MessageWriter result;
    std::uint8_t * pos = output;
    *(pos++) = version;
    *(pos++) = COUNTER_TAG;
    pos = varint_encode(pos, counter);
    *(pos++) = RATCHET_KEY_TAG;
    pos = varint_encode(pos, ratchet_key_length);
    result.ratchet_key = pos;
    pos += ratchet_key_length;
    *(pos++) = CIPHERTEXT_TAG;
    pos = varint_encode(pos, ciphertext_length);
    result.ciphertext = pos;
    pos += ciphertext_length;
    result.body_length = pos - output;
    result.mac = pos;
    return result;
}


axolotl::MessageReader axolotl::decode_message(
    std::uint8_t const * input, std::size_t input_length,
    std::size_t mac_length
) {
    axolotl::MessageReader result;
    result.body_length = 0;
    std::uint8_t const * pos = input;
    std::uint8_t const * end = input + input_length - mac_length;
    std::uint8_t flags = 0;
    result.mac = end;
    if (pos == end) return result;
    result.version = *(pos++);
    while (pos != end) {
        uint8_t tag = *(pos);
        if (tag == COUNTER_TAG) {
            ++pos;
            std::uint8_t const * counter_start = pos;
            pos = varint_skip(pos, end);
            result.counter = varint_decode<std::uint32_t>(counter_start, pos);
            flags |= 1;
        } else if (tag == RATCHET_KEY_TAG) {
            ++pos;
            std::uint8_t const * len_start = pos;
            pos = varint_skip(pos, end);
            std::size_t len = varint_decode<std::size_t>(len_start, pos);
            if (len > end - pos) return result;
            result.ratchet_key_length = len;
            result.ratchet_key = pos;
            pos += len;
            flags |= 2;
        } else if (tag == CIPHERTEXT_TAG) {
            ++pos;
            std::uint8_t const * len_start = pos;
            pos = varint_skip(pos, end);
            std::size_t len = varint_decode<std::size_t>(len_start, pos);
            if (len > end - pos) return result;
            result.ciphertext_length = len;
            result.ciphertext = pos;
            pos += len;
            flags |= 4;
        } else if (tag & 0x7 == 0) {
            pos = varint_skip(pos, end);
            pos = varint_skip(pos, end);
        } else if (tag & 0x7 == 2) {
            std::uint8_t const * len_start = pos;
            pos = varint_skip(pos, end);
            std::size_t len = varint_decode<std::size_t>(len_start, pos);
            if (len > end - pos) return result;
            pos += len;
        } else {
            return result;
        }
    }
    if (flags == 0x7) {
        result.body_length = end - input;
    }
    return result;
}
