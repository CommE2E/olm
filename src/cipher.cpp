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
#include "olm/cipher.hh"
#include "olm/crypto.hh"
#include "olm/memory.hh"
#include <cstring>

olm::Cipher::~Cipher() {

}

namespace {

struct DerivedKeys {
    olm::Aes256Key aes_key;
    std::uint8_t mac_key[olm::KEY_LENGTH];
    olm::Aes256Iv aes_iv;
};


static void derive_keys(
    std::uint8_t const * kdf_info, std::size_t kdf_info_length,
    std::uint8_t const * key, std::size_t key_length,
    DerivedKeys & keys
) {
    std::uint8_t derived_secrets[2 * olm::KEY_LENGTH + olm::IV_LENGTH];
    olm::hkdf_sha256(
        key, key_length,
        nullptr, 0,
        kdf_info, kdf_info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    std::uint8_t const * pos = derived_secrets;
    pos = olm::load_array(keys.aes_key.key, pos);
    pos = olm::load_array(keys.mac_key, pos);
    pos = olm::load_array(keys.aes_iv.iv, pos);
    olm::unset(derived_secrets);
}

static const std::size_t MAC_LENGTH = 8;

} // namespace


olm::CipherAesSha256::CipherAesSha256(
    std::uint8_t const * kdf_info, std::size_t kdf_info_length
) : kdf_info(kdf_info), kdf_info_length(kdf_info_length) {

}


std::size_t olm::CipherAesSha256::mac_length() const {
    return MAC_LENGTH;
}


std::size_t olm::CipherAesSha256::encrypt_ciphertext_length(
    std::size_t plaintext_length
) const {
    return olm::aes_encrypt_cbc_length(plaintext_length);
}


std::size_t olm::CipherAesSha256::encrypt(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t const * plaintext, std::size_t plaintext_length,
    std::uint8_t * ciphertext, std::size_t ciphertext_length,
    std::uint8_t * output, std::size_t output_length
) const {
    if (encrypt_ciphertext_length(plaintext_length) < ciphertext_length) {
        return std::size_t(-1);
    }
    struct DerivedKeys keys;
    std::uint8_t mac[olm::SHA256_OUTPUT_LENGTH];

    derive_keys(kdf_info, kdf_info_length, key, key_length, keys);

    olm::aes_encrypt_cbc(
        keys.aes_key, keys.aes_iv, plaintext, plaintext_length, ciphertext
    );

    olm::hmac_sha256(
        keys.mac_key, olm::KEY_LENGTH, output, output_length - MAC_LENGTH, mac
    );

    std::memcpy(output + output_length - MAC_LENGTH, mac, MAC_LENGTH);

    olm::unset(keys);
    return output_length;
}


std::size_t olm::CipherAesSha256::decrypt_max_plaintext_length(
    std::size_t ciphertext_length
) const {
    return ciphertext_length;
}

std::size_t olm::CipherAesSha256::decrypt(
     std::uint8_t const * key, std::size_t key_length,
     std::uint8_t const * input, std::size_t input_length,
     std::uint8_t const * ciphertext, std::size_t ciphertext_length,
     std::uint8_t * plaintext, std::size_t max_plaintext_length
) const {
    DerivedKeys keys;
    std::uint8_t mac[olm::SHA256_OUTPUT_LENGTH];

    derive_keys(kdf_info, kdf_info_length, key, key_length, keys);

    olm::hmac_sha256(
        keys.mac_key, olm::KEY_LENGTH, input, input_length - MAC_LENGTH, mac
    );

    std::uint8_t const * input_mac = input + input_length - MAC_LENGTH;
    if (!olm::is_equal(input_mac, mac, MAC_LENGTH)) {
        olm::unset(keys);
        return std::size_t(-1);
    }

    std::size_t plaintext_length = olm::aes_decrypt_cbc(
        keys.aes_key, keys.aes_iv, ciphertext, ciphertext_length, plaintext
    );

    olm::unset(keys);
    return plaintext_length;
}
