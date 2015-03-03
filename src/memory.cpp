#include "axolotl/memory.hh"


void axolotl::unset(
    void volatile * buffer, std::size_t buffer_length
) {
    char volatile * pos = reinterpret_cast<char volatile *>(buffer);
    char volatile * end = pos + buffer_length;
    while (pos != end) {
        *(pos++) = 0;
    }
}


bool axolotl::is_equal(
    std::uint8_t const * buffer_a,
    std::uint8_t const * buffer_b,
    std::size_t length
) {
    std::uint8_t volatile result = 0;
    while (length--) {
        result |= (*(buffer_a++)) ^ (*(buffer_b++));
    }
    return result == 0;
}
