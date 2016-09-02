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
