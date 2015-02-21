#include <cstdint>

namespace axolotl {


struct Curve25519PublicKey {
    static const int LENGTH = 32;
    std::uint8_t public_key[32];
};


struct Curve25519KeyPair : public Curve25519PublicKey {
    std::uint8_t private_key[32];
};


Curve25519KeyPair generate_key(
    std::uint8_t const * random_32_bytes
);


const std::size_t CURVE25519_SHARED_SECRET_LENGTH = 32;


void curve25519_shared_secret(
    Curve25519KeyPair const & our_key,
    Curve25519PublicKey const & their_key,
    std::uint8_t * output
);


struct Aes256Key {
    static const int LENGTH = 32;
    std::uint8_t key[32];
};


struct Aes256Iv {
    static const int LENGTH = 16;
    std::uint8_t iv[16];
};


std::size_t aes_pkcs_7_padded_length(
    std::size_t input_length
);


void aes_pkcs_7_padding(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


void aes_encrypt_cbc(
    Aes256Key const & key,
    Aes256Iv const & iv,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


void aes_decrypt_cbc(
    Aes256Key const & key,
    Aes256Iv const & iv,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


void sha256(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


const std::size_t HMAC_SHA256_OUTPUT_LENGTH = 32;


void hmac_sha256(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


void hkdf_sha256(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t const * info, std::size_t info_length,
    std::uint8_t const * salt, std::size_t salt_length,
    std::uint8_t * output, std::size_t output_length
);

} // namespace axolotl
