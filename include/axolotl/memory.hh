#include <cstddef>

namespace axolotl {

/** Clear the memory held in the buffer */
void unset(
    volatile void * buffer, std::size_t buffer_length
);

/** Clear the memory backing an object */
template<typename T>
void unset(T & value) {
    unset(reinterpret_cast<volatile void *>(&value), sizeof(T));
}


} // namespace axolotl
