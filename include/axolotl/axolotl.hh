
#include "axolotl/crypto.hh"
#include "axolotl/list.hh"

namespace axolotl {

typedef std::uint8_t SharedKey[32];


struct ChainKey {
    std::uint32_t index;
    SharedKey key;
};


struct MessageKey {
    std::uint32_t index;
    Aes256Key cipher_key;
    SharedKey mac_key;
    Aes256Iv iv;
};


struct SenderChain {
    Curve25519KeyPair ratchet_key;
    ChainKey chain_key;
};


struct ReceiverChain {
    Curve25519PublicKey ratchet_key;
    ChainKey chain_key;
};


struct SkippedMessageKey {
    Curve25519PublicKey ratchet_key;
    MessageKey message_key;
};


enum struct ErrorCode {
    SUCCESS = 0, /*!< There wasn't an error */
    NOT_ENOUGH_RANDOM = 1,  /*!< Not enough entropy was supplied */
    OUTPUT_BUFFER_TOO_SMALL = 2, /*!< Supplied output buffer is too small */
    BAD_MESSAGE_VERSION = 3,  /*!< The message version is unsupported */
    BAD_MESSAGE_FORMAT = 4, /*!< The message couldn't be decoded */
    BAD_MESSAGE_MAC = 5, /*!< The message couldn't be decrypted */
};


static std::size_t const MAX_RECEIVER_CHAINS = 5;
static std::size_t const MAX_SKIPPED_MESSAGE_KEYS = 40;


struct KdfInfo {
    std::uint8_t const * root_info;
    std::size_t root_info_length;
    std::uint8_t const * ratchet_info;
    std::size_t ratchet_info_length;
    std::uint8_t const * message_info;
    std::size_t message_info_length;
};


struct Session {

    Session(
        KdfInfo const & kdf_info
    );

    /** A pair of string to feed into the KDF identifing the application */
    KdfInfo kdf_info;
    /** The last error that happened encypting or decrypting a message */
    ErrorCode last_error;
    SharedKey root_key;
    List<SenderChain, 1> sender_chain;
    List<ReceiverChain, MAX_RECEIVER_CHAINS> receiver_chains;
    List<SkippedMessageKey, MAX_SKIPPED_MESSAGE_KEYS> skipped_message_keys;

    void initialise_as_bob(
        std::uint8_t const * shared_secret, std::size_t shared_secret_length,
        Curve25519PublicKey const & their_ratchet_key
    );

    void initialise_as_alice(
        std::uint8_t const * shared_secret, std::size_t shared_secret_length,
        Curve25519KeyPair const & our_ratchet_key
    );

    std::size_t encrypt_max_output_length(
        std::size_t plaintext_length
    );

    std::size_t encrypt_random_length();

    std::size_t encrypt(
        std::uint8_t const * plaintext, std::size_t plaintext_length,
        std::uint8_t const * random, std::size_t random_length,
        std::uint8_t * output, std::size_t max_output_length
    );

    std::size_t decrypt_max_plaintext_length(
        std::size_t input_length
    );

    std::size_t decrypt(
        std::uint8_t const * input, std::size_t input_length,
        std::uint8_t * plaintext, std::size_t max_plaintext_length
    );
};


} // namespace axololt
