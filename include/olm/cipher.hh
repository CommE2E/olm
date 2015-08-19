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

#ifndef OLM_CIPHER_HH_
#define OLM_CIPHER_HH_

#include <cstdint>
#include <cstddef>

namespace olm {

class Cipher {
public:
    virtual ~Cipher();

    /**
     * Returns the length of the message authentication code that will be
     * appended to the output.
     */
    virtual std::size_t mac_length() const = 0;

    /**
     * Returns the length of cipher-text for a given length of plain-text.
     */
    virtual std::size_t encrypt_ciphertext_length(
        std::size_t plaintext_length
    ) const = 0;

    /*
     * Encrypts the plain-text into the output buffer and authenticates the
     * contents of the output buffer covering both cipher-text and any other
     * associated data in the output buffer.
     *
     *  |---------------------------------------output_length-->|
     *  output  |--ciphertext_length-->|       |---mac_length-->|
     *          ciphertext
     *
     * The plain-text pointers and cipher-text pointers may be the same.
     *
     * Returns std::size_t(-1) if the length of the cipher-text or the output
     * buffer is too small. Otherwise returns the length of the output buffer.
     */
    virtual std::size_t encrypt(
        std::uint8_t const * key, std::size_t key_length,
        std::uint8_t const * plaintext, std::size_t plaintext_length,
        std::uint8_t * ciphertext, std::size_t ciphertext_length,
        std::uint8_t * output, std::size_t output_length
    ) const = 0;

    /**
     * Returns the maximum length of plain-text that a given length of
     * cipher-text can contain.
     */
    virtual std::size_t decrypt_max_plaintext_length(
        std::size_t ciphertext_length
    ) const = 0;

    /**
     * Authenticates the input and decrypts the cipher-text into the plain-text
     * buffer.
     *
     *  |----------------------------------------input_length-->|
     *  input   |--ciphertext_length-->|       |---mac_length-->|
     *          ciphertext
     *
     * The plain-text pointers and cipher-text pointers may be the same.
     *
     *  Returns std::size_t(-1) if the length of the plain-text buffer is too
     *  small or if the authentication check fails. Otherwise returns the length
     *  of the plain text.
     */
    virtual std::size_t decrypt(
       std::uint8_t const * key, std::size_t key_length,
       std::uint8_t const * input, std::size_t input_length,
       std::uint8_t const * ciphertext, std::size_t ciphertext_length,
       std::uint8_t * plaintext, std::size_t max_plaintext_length
    ) const = 0;
};


class CipherAesSha256 : public Cipher {
public:
    CipherAesSha256(
        std::uint8_t const * kdf_info, std::size_t kdf_info_length
    );

    virtual std::size_t mac_length() const;

    virtual std::size_t encrypt_ciphertext_length(
        std::size_t plaintext_length
    ) const;

    virtual std::size_t encrypt(
        std::uint8_t const * key, std::size_t key_length,
        std::uint8_t const * plaintext, std::size_t plaintext_length,
        std::uint8_t * ciphertext, std::size_t ciphertext_length,
        std::uint8_t * output, std::size_t output_length
    ) const;

    virtual std::size_t decrypt_max_plaintext_length(
        std::size_t ciphertext_length
    ) const;

    virtual std::size_t decrypt(
        std::uint8_t const * key, std::size_t key_length,
        std::uint8_t const * input, std::size_t input_length,
        std::uint8_t const * ciphertext, std::size_t ciphertext_length,
        std::uint8_t * plaintext, std::size_t max_plaintext_length
    ) const;

private:
    std::uint8_t const * kdf_info;
    std::size_t kdf_info_length;
};


} // namespace


#endif /* OLM_CIPHER_HH_ */
