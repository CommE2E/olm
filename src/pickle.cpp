#include "axolotl/pickle.hh"


std::size_t axolotl::pickle_length(
    const axolotl::Curve25519PublicKey & value
) {
    return sizeof(value.public_key);
}


std::uint8_t * axolotl::pickle(
    std::uint8_t * pos,
    const axolotl::Curve25519PublicKey & value
) {
    pos = axolotl::pickle_bytes(
        pos, value.public_key, sizeof(value.public_key)
    );
    return pos;
}


std::uint8_t const * axolotl::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::Curve25519PublicKey & value
) {
    pos = axolotl::unpickle_bytes(
        pos, end, value.public_key, sizeof(value.public_key)
    );
    return pos;

}


std::size_t axolotl::pickle_length(
    const axolotl::Curve25519KeyPair & value
) {
    return sizeof(value.public_key) + sizeof(value.private_key);
}


std::uint8_t * axolotl::pickle(
    std::uint8_t * pos,
    const axolotl::Curve25519KeyPair & value
) {
    pos = axolotl::pickle_bytes(
        pos, value.public_key, sizeof(value.public_key)
    );
    pos = axolotl::pickle_bytes(
        pos, value.private_key, sizeof(value.private_key)
    );
    return pos;
}


std::uint8_t const * axolotl::unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    axolotl::Curve25519KeyPair & value
) {
    pos = axolotl::unpickle_bytes(
        pos, end, value.public_key, sizeof(value.public_key)
    );
    pos = axolotl::unpickle_bytes(
        pos, end, value.private_key, sizeof(value.private_key)
    );
    return pos;
}
