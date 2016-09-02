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

/* C-compatible crpyto utility functions. At some point all of crypto.hh will
 * move here.
 */

#ifndef OLM_CRYPTO_H_
#define OLM_CRYPTO_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** length of a sha256 hash */
#define SHA256_OUTPUT_LENGTH 32

/** length of a public or private Curve25519 key */
#define CURVE25519_KEY_LENGTH 32

/** length of the shared secret created by a Curve25519 ECDH operation */
#define CURVE25519_SHARED_SECRET_LENGTH 32

/** amount of random data required to create a Curve25519 keypair */
#define CURVE25519_RANDOM_LENGTH CURVE25519_KEY_LENGTH

/** length of a public Ed25519 key */
#define ED25519_PUBLIC_KEY_LENGTH 32

/** length of a private Ed25519 key */
#define ED25519_PRIVATE_KEY_LENGTH 64

/** amount of random data required to create a Ed25519 keypair */
#define ED25519_RANDOM_LENGTH 32

/** length of an Ed25519 signature */
#define ED25519_SIGNATURE_LENGTH 64

/** length of an aes256 key */
#define AES256_KEY_LENGTH 32

/** length of an aes256 initialisation vector */
#define AES256_IV_LENGTH 16


/** Computes SHA-256 of the input. The output buffer must be a least 32
 * bytes long. */
void _olm_crypto_sha256(
    uint8_t const * input, size_t input_length,
    uint8_t * output
);

/** HMAC: Keyed-Hashing for Message Authentication
 * http://tools.ietf.org/html/rfc2104
 * Computes HMAC-SHA-256 of the input for the key. The output buffer must
 * be at least 32 bytes long. */
void _olm_crypto_hmac_sha256(
    uint8_t const * key, size_t key_length,
    uint8_t const * input, size_t input_length,
    uint8_t * output
);


/** HMAC-based Key Derivation Function (HKDF)
 * https://tools.ietf.org/html/rfc5869
 * Derives key material from the input bytes. */
void _olm_crypto_hkdf_sha256(
    uint8_t const * input, size_t input_length,
    uint8_t const * info, size_t info_length,
    uint8_t const * salt, size_t salt_length,
    uint8_t * output, size_t output_length
);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OLM_CRYPTO_H_ */
