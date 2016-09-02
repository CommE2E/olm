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
#ifndef OLM_CRYPTO_HH_
#define OLM_CRYPTO_HH_

#include <cstdint>
#include <cstddef>

// eventually all of this needs to move into crypto.h, and everything should
// use that. For now, include crypto.h here.

#include "olm/crypto.h"

namespace olm {

struct Curve25519PublicKey {
    std::uint8_t public_key[CURVE25519_KEY_LENGTH];
};


struct Curve25519KeyPair : public Curve25519PublicKey {
    std::uint8_t private_key[CURVE25519_KEY_LENGTH];
};


struct Ed25519PublicKey {
    std::uint8_t public_key[ED25519_PUBLIC_KEY_LENGTH];
};


struct Ed25519KeyPair : public Ed25519PublicKey {
    std::uint8_t private_key[ED25519_PRIVATE_KEY_LENGTH];
};


/** Generate a curve25519 key pair from 32 random bytes. */
void curve25519_generate_key(
    std::uint8_t const * random_32_bytes,
    Curve25519KeyPair & key_pair
);


/** Create a shared secret using our private key and their public key.
 * The output buffer must be at least 32 bytes long. */
void curve25519_shared_secret(
    Curve25519KeyPair const & our_key,
    Curve25519PublicKey const & their_key,
    std::uint8_t * output
);


/** Generate a curve25519 key pair from 32 random bytes. */
void ed25519_generate_key(
    std::uint8_t const * random_32_bytes,
    Ed25519KeyPair & key_pair
);


/** Signs the message using our private key.
 * The output buffer must be at least 64 bytes long. */
void ed25519_sign(
    Ed25519KeyPair const & our_key,
    std::uint8_t const * message, std::size_t message_length,
    std::uint8_t * output
);


/** Verify their message using their public key.
 * The signature input buffer must be 64 bytes long.
 * Returns true if the signature is valid. */
bool ed25519_verify(
    Ed25519PublicKey const & their_key,
    std::uint8_t const * message, std::size_t message_length,
    std::uint8_t const * signature
);


struct Aes256Key {
    std::uint8_t key[AES256_KEY_LENGTH];
};


struct Aes256Iv {
    std::uint8_t iv[AES256_IV_LENGTH];
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


} // namespace olm

#endif /* OLM_CRYPTO_HH_ */
