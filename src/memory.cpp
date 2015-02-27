#include "axolotl/memory.hh"

void axolotl::unset(
    volatile void * buffer, std::size_t buffer_length
) {
    volatile char * pos = reinterpret_cast<volatile char *>(buffer);
    volatile char * end = pos + buffer_length;
    while (pos != end) {
        *(pos++) = 0;
    }
}
