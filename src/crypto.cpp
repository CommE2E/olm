#include "axolotl/crypto.hh"
#include <cstring>

extern "C" {

int curve25519_donna(
    uint8_t * output,
    const uint8_t * secret,
    const uint8_t * basepoint
);

#include "crypto-algorithms/aes.h"
#include "crypto-algorithms/sha256.h"

}


namespace {

static const std::uint8_t CURVE25519_BASEPOINT[32] = {9};
static const std::size_t AES_BLOCK_LENGTH = 16;
static const std::size_t SHA256_HASH_LENGTH = 32;
static const std::size_t SHA256_BLOCK_LENGTH = 64;
static const std::uint8_t HKDF_DEFAULT_SALT[32] = {};

template<std::size_t block_size>
inline static void xor_block(
    std::uint8_t * block,
    std::uint8_t const * input
) {
    for (std::size_t i = 0; i < block_size; ++i) {
        block[i] ^= input[i];
    }
}


inline static void hmac_sha256_key(
    std::uint8_t const * input_key, std::size_t input_key_length,
    std::uint8_t * hmac_key
) {
    std::memset(hmac_key, 0, SHA256_BLOCK_LENGTH);
    if (input_key_length > SHA256_BLOCK_LENGTH) {
        ::SHA256_CTX context;
        ::sha256_init(&context);
        ::sha256_update(&context, input_key, input_key_length);
        ::sha256_final(&context, hmac_key);
    } else {
        std::memcpy(hmac_key, input_key, input_key_length);
    }
}


inline void hmac_sha256_init(
    ::SHA256_CTX * context,
    std::uint8_t const * hmac_key
) {
    std::uint8_t i_pad[SHA256_BLOCK_LENGTH];
    std::memcpy(i_pad, hmac_key, SHA256_BLOCK_LENGTH);
    for (std::size_t i = 0; i < SHA256_BLOCK_LENGTH; ++i) {
        i_pad[i] ^= 0x36;
    }
    ::sha256_init(context);
    ::sha256_update(context, i_pad, SHA256_BLOCK_LENGTH);
    std::memset(i_pad, 0, sizeof(i_pad));
}


inline void hmac_sha256_final(
    ::SHA256_CTX * context,
    std::uint8_t const * hmac_key,
    std::uint8_t * output
) {
    std::uint8_t o_pad[SHA256_BLOCK_LENGTH + SHA256_HASH_LENGTH];
    std::memcpy(o_pad, hmac_key, SHA256_BLOCK_LENGTH);
    for (std::size_t i = 0; i < SHA256_BLOCK_LENGTH; ++i) {
        o_pad[i] ^= 0x5C;
    }
    ::sha256_final(context, o_pad + SHA256_BLOCK_LENGTH);
    ::SHA256_CTX final_context;
    ::sha256_init(&final_context);
    ::sha256_update(&final_context, o_pad, sizeof(o_pad));
    ::sha256_final(&final_context, output);
    std::memset(o_pad, 0, sizeof(o_pad));
}

} // namespace


axolotl::Curve25519KeyPair axolotl::generate_key(
    std::uint8_t const * random_32_bytes
) {
    axolotl::Curve25519KeyPair key_pair;
    std::memcpy(key_pair.private_key, random_32_bytes, 32);
    ::curve25519_donna(
        key_pair.public_key, key_pair.private_key, CURVE25519_BASEPOINT
    );
    return key_pair;
}


void axolotl::curve25519_shared_secret(
    axolotl::Curve25519KeyPair const & our_key,
    axolotl::Curve25519PublicKey const & their_key,
    std::uint8_t * output
) {
    ::curve25519_donna(output, our_key.private_key, their_key.public_key);
}


std::size_t axolotl::aes_pkcs_7_padded_length(
    std::size_t input_length
) {
    return input_length + AES_BLOCK_LENGTH - input_length % AES_BLOCK_LENGTH;
}


void axolotl::aes_pkcs_7_padding(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
) {
    std::memcpy(output, input, input_length);
    std::size_t padded_length = axolotl::aes_pkcs_7_padded_length(input_length);
    std::uint8_t padding = padded_length - input_length;
    for (std::size_t i = input_length; i < padded_length; ++i) {
        output[i] = padding;
    }
}


void axolotl::aes_encrypt_cbc(
    axolotl::Aes256Key const & key,
    axolotl::Aes256Iv const & iv,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
) {
    std::uint32_t key_schedule[60];
    ::aes_key_setup(key.key, key_schedule, 256);
    std::uint8_t input_block[AES_BLOCK_LENGTH];
    std::memcpy(input_block, iv.iv, AES_BLOCK_LENGTH);
    for (std::size_t i = 0; i < input_length; i += AES_BLOCK_LENGTH) {
        xor_block<AES_BLOCK_LENGTH>(input_block, &input[i]);
        ::aes_encrypt(input_block, &output[i], key_schedule, 256);
        std::memcpy(input_block, &output[i], AES_BLOCK_LENGTH);
    }
    std::memset(key_schedule, 0, sizeof(key_schedule));
    std::memset(input_block, 0, sizeof(AES_BLOCK_LENGTH));
}


void axolotl::aes_decrypt_cbc(
    axolotl::Aes256Key const & key,
    axolotl::Aes256Iv const & iv,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
) {
    std::uint32_t key_schedule[60];
    ::aes_key_setup(key.key, key_schedule, 256);
    for (std::size_t i = 0; i < input_length; i += AES_BLOCK_LENGTH) {
        ::aes_decrypt(&input[i], &output[i], key_schedule, 256);
        if (i == 0) {
            xor_block<AES_BLOCK_LENGTH>(&output[i], iv.iv);
        } else {
            xor_block<AES_BLOCK_LENGTH>(&output[i], &input[i - AES_BLOCK_LENGTH]);
        }
    }
    std::memset(key_schedule, 0, sizeof(key_schedule));
}


void axolotl::sha256(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
) {
    ::SHA256_CTX context;
    ::sha256_init(&context);
    ::sha256_update(&context, input, input_length);
    ::sha256_final(&context, output);
}

void axolotl::hmac_sha256(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
) {
    std::uint8_t hmac_key[SHA256_BLOCK_LENGTH];
    ::SHA256_CTX context;
    hmac_sha256_key(key, key_length, hmac_key);
    hmac_sha256_init(&context, hmac_key);
    ::sha256_update(&context, input, input_length);
    hmac_sha256_final(&context, hmac_key, output);
    std::memset(hmac_key, 0, sizeof(hmac_key));
}


void axolotl::hkdf_sha256(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t const * salt, std::size_t salt_length,
    std::uint8_t const * info, std::size_t info_length,
    std::uint8_t * output, std::size_t output_length
) {
    ::SHA256_CTX context;
    std::uint8_t hmac_key[SHA256_BLOCK_LENGTH];
    std::uint8_t step_result[SHA256_HASH_LENGTH];
    std::size_t bytes_remaining = output_length;
    std::uint8_t iteration = 1;
    if (!salt) {
        salt = HKDF_DEFAULT_SALT;
        salt_length = sizeof(HKDF_DEFAULT_SALT);
    }
    /* Expand */
    hmac_sha256_key(salt, salt_length, hmac_key);
    hmac_sha256_init(&context, hmac_key);
    ::sha256_update(&context, input, input_length);
    hmac_sha256_final(&context, hmac_key, step_result);
    hmac_sha256_key(step_result, SHA256_HASH_LENGTH, hmac_key);

    /* Extract */
    hmac_sha256_init(&context, hmac_key);
    ::sha256_update(&context, info, info_length);
    ::sha256_update(&context, &iteration, 1);
    hmac_sha256_final(&context, hmac_key, step_result);
    while (bytes_remaining > SHA256_HASH_LENGTH) {
        std::memcpy(output, step_result, SHA256_HASH_LENGTH);
        output += SHA256_HASH_LENGTH;
        bytes_remaining -= SHA256_HASH_LENGTH;
        iteration ++;
        hmac_sha256_init(&context, hmac_key);
        ::sha256_update(&context, step_result, SHA256_HASH_LENGTH);
        ::sha256_update(&context, info, info_length);
        ::sha256_update(&context, &iteration, 1);
        hmac_sha256_final(&context, hmac_key, step_result);
    }
    std::memcpy(output, step_result, bytes_remaining);
}
