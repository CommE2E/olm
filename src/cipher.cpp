#include "axolotl/cipher.hh"
#include "axolotl/crypto.hh"
#include "axolotl/memory.hh"
#include <cstring>

axolotl::Cipher::~Cipher() {

}

namespace {

static const std::size_t SHA256_LENGTH = 32;

struct DerivedKeys {
    axolotl::Aes256Key aes_key;
    std::uint8_t mac_key[SHA256_LENGTH];
    axolotl::Aes256Iv aes_iv;
};


static void derive_keys(
    std::uint8_t const * kdf_info, std::size_t kdf_info_length,
    std::uint8_t const * key, std::size_t key_length,
    DerivedKeys & keys
) {
    std::uint8_t derived_secrets[80];
    axolotl::hkdf_sha256(
        key, key_length,
        NULL, 0,
        kdf_info, kdf_info_length,
        derived_secrets, sizeof(derived_secrets)
    );
    std::memcpy(keys.aes_key.key, derived_secrets, 32);
    std::memcpy(keys.mac_key, derived_secrets + 32, 32);
    std::memcpy(keys.aes_iv.iv, derived_secrets + 64, 16);
    axolotl::unset(derived_secrets);
}

static const std::size_t MAC_LENGTH = 8;

} // namespace


axolotl::CipherAesSha256::CipherAesSha256(
    std::uint8_t const * kdf_info, std::size_t kdf_info_length
) : kdf_info(kdf_info), kdf_info_length(kdf_info_length) {

}


std::size_t axolotl::CipherAesSha256::mac_length() const {
    return MAC_LENGTH;
}


std::size_t axolotl::CipherAesSha256::encrypt_ciphertext_length(
    std::size_t plaintext_length
) const {
    return axolotl::aes_encrypt_cbc_length(plaintext_length);
}


std::size_t axolotl::CipherAesSha256::encrypt(
    std::uint8_t const * key, std::size_t key_length,
    std::uint8_t const * plaintext, std::size_t plaintext_length,
    std::uint8_t * ciphertext, std::size_t ciphertext_length,
    std::uint8_t * output, std::size_t output_length
) const {
    if (encrypt_ciphertext_length(plaintext_length) < ciphertext_length) {
        return std::size_t(-1);
    }
    struct DerivedKeys keys;
    std::uint8_t mac[SHA256_LENGTH];

    derive_keys(kdf_info, kdf_info_length, key, key_length, keys);

    axolotl::aes_encrypt_cbc(
        keys.aes_key, keys.aes_iv, plaintext, plaintext_length, ciphertext
    );

    axolotl::hmac_sha256(
        keys.mac_key, SHA256_LENGTH, output, output_length - MAC_LENGTH, mac
    );

    std::memcpy(output + output_length - MAC_LENGTH, mac, MAC_LENGTH);

    axolotl::unset(keys);
    return output_length;
}


std::size_t axolotl::CipherAesSha256::decrypt_max_plaintext_length(
    std::size_t ciphertext_length
) const {
    return ciphertext_length;
}

std::size_t axolotl::CipherAesSha256::decrypt(
     std::uint8_t const * key, std::size_t key_length,
     std::uint8_t const * input, std::size_t input_length,
     std::uint8_t const * ciphertext, std::size_t ciphertext_length,
     std::uint8_t * plaintext, std::size_t max_plaintext_length
) const {
    DerivedKeys keys;
    std::uint8_t mac[SHA256_LENGTH];

    derive_keys(kdf_info, kdf_info_length, key, key_length, keys);

    axolotl::hmac_sha256(
        keys.mac_key, SHA256_LENGTH, input, input_length - MAC_LENGTH, mac
    );

    std::uint8_t const * input_mac = input + input_length - MAC_LENGTH;
    if (!axolotl::is_equal(input_mac, mac, MAC_LENGTH)) {
        axolotl::unset(keys);
        return std::size_t(-1);
    }

    std::size_t plaintext_length = axolotl::aes_decrypt_cbc(
        keys.aes_key, keys.aes_iv, ciphertext, ciphertext_length, plaintext
    );

    axolotl::unset(keys);
    return plaintext_length;
}
