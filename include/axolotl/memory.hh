#include <cstddef>
#include <cstdint>

namespace axolotl {

/** Clear the memory held in the buffer */
void unset(
    void volatile * buffer, std::size_t buffer_length
);

/** Clear the memory backing an object */
template<typename T>
void unset(T & value) {
    unset(reinterpret_cast<void volatile *>(&value), sizeof(T));
}

/** Check if two buffers are equal in constant time. */
bool is_equal(
    std::uint8_t const * buffer_a,
    std::uint8_t const * buffer_b,
    std::size_t length
);

} // namespace axolotl
