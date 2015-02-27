/* Copyright 2015 OpenMarket Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdint>
#include <cstddef>

namespace axolotl {


struct Curve25519PublicKey {
    static const int LENGTH = 32;
    std::uint8_t public_key[32];
};


struct Curve25519KeyPair : public Curve25519PublicKey {
    std::uint8_t private_key[32];
};

/** Generate a curve25519 key pair from 32 random bytes. */
void generate_key(
    std::uint8_t const * random_32_bytes,
    Curve25519KeyPair & key_pair
);


const std::size_t CURVE25519_SHARED_SECRET_LENGTH = 32;

/** Create a shared secret using our private key and their public key.
 * The output buffer must be at least 32 bytes long. */
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


/** The length of output the aes_encrypt_cbc function will write */
std::size_t aes_encrypt_cbc_length(
    std::size_t input_length
);


/** Encrypts the input using AES256 in CBC mode with PKCS#7 padding.
 * The output buffer must be big enough to hold the output including padding */
void aes_encrypt_cbc(
    Aes256Key const & key,
    Aes256Iv const & iv,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


/** Decrypts the input using AES256 in CBC mode. The output buffer must be at
 * least the same size as the input buffer. Returns the length of the plaintext
 * without padding on success or std::size_t(-1) if the padding is invalid.
 */
std::size_t aes_decrypt_cbc(
    Aes256Key const & key,
    Aes256Iv const & iv,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


/** Computes SHA-256 of the input. The output buffer must be a least 32
 * bytes long. */
void sha256(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


const std::size_t HMAC_SHA256_OUTPUT_LENGTH = 32;


/** HMAC: Keyed-Hashing for Message Authentication
 * http://tools.ietf.org/html/rfc2104
 * Computes HMAC-SHA-256 of the input for the key. The output buffer must
 * be at least 32 bytes long. */
void hmac_sha256(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);


/** HMAC-based Key Derivation Function (HKDF)
 * https://tools.ietf.org/html/rfc5869
 * Derives key material from the input bytes. */
void hkdf_sha256(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t const * info, std::size_t info_length,
    std::uint8_t const * salt, std::size_t salt_length,
    std::uint8_t * output, std::size_t output_length
);

} // namespace axolotl
