#ifndef AXOLOTL_SESSION_HH_
#define AXOLOTL_SESSION_HH_

#include "axolotl/ratchet.hh"

namespace axolotl {

struct RemoteKey {
    std::uint32_t id;
    Curve25519PublicKey key;
};

struct RemoteKeys {
};


enum struct MessageType {
    PRE_KEY_MESSAGE = 0,
    MESSAGE = 1,
};


struct Session {
    bool received_message;
    RemoteKey alice_identity_key;
    RemoteKey alice_base_key;
    RemoteKey bob_identity_key;
    RemoteKey bob_one_time_key;
    Ratchet ratchet;

    void initialise_outbound_session_random_length();

    void initialise_outbound_session(
        Account const & local_account,
        RemoteKey const & identity_key,
        RemoteKey const & one_time_key,
        std::uint8_t const * random, std::size_t random_length
    );

    void initialise_inbound_session(
        Account & local_account,
        std::uint8_t const * one_time_key_message, std::size_t message_length
    );

    void matches_inbound_session(
        std::uint8_t const * one_time_key_message, std::size_t message_length
    );

    MessageType encrypt_message_type();

    std::size_t encrypt_message_length(
        std::size_t plaintext_length
    );

    std::size_t encrypt_random_length();

    std::size_t encrypt(
        std::uint8_t const * plaintext, std::size_t plaintext_length,
        std::uint8_t const * random, std::size_t random_length,
        std::uint8_t * message, std::size_t message_length
    );

    std::size_t decrypt_max_plaintext_length(
        MessageType message_type,
        std::uint8_t const * message, std::size_t message_length
    );

    std::size_t decrypt(
        MessageType message_type,
        std::uint8_t const * message, std::size_t message_length,
        std::uint8_t * plaintext, std::size_t max_plaintext_length
    );
};


} // namespace axolotl

#endif /* AXOLOTL_SESSION_HH_ */
