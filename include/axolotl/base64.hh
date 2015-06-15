#ifndef AXOLOLT_BASE64_HH_
#define AXOLOLT_BASE64_HH_

#include <cstddef>
#include <cstdint>

namespace axolotl {


std::size_t encode_base64_length(
    std::size_t input_length
);


void encode_base64(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


std::size_t decode_base64_length(
    std::size_t input_length
);


void decode_base64(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


} // namespace axolotl


#endif /* AXOLOLT_BASE64_HH_ */
