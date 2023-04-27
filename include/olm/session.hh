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
#ifndef OLM_SESSION_HH_
#define OLM_SESSION_HH_

#include "olm/ratchet.hh"

// Note: exports in this file are only for unit tests.  Nobody else should be
// using this externally
#include "olm/olm_export.h"

namespace olm {

struct Account;

enum struct MessageType {
    PRE_KEY = 0,
    MESSAGE = 1,
};

struct OLM_EXPORT Session {

    Session();

    Ratchet ratchet;
    OlmErrorCode last_error;

    bool received_message;

    _olm_curve25519_public_key alice_identity_key;
    _olm_curve25519_public_key alice_base_key;
    _olm_curve25519_public_key bob_one_time_key;
    _olm_curve25519_public_key bob_prekey;

    /** The number of random bytes that are needed to create a new outbound
     * session. This will be 64 bytes since two ephemeral keys are needed. */
    std::size_t new_outbound_session_random_length() const;

    /** Start a new outbound session. Returns std::size_t(-1) on failure. Assumes
     * prekey_signature is a byte array of ED25519_SIGNATURE_LENGTH characters. On
     * failure last_error will be set with an error code. The last_error will be
     * NOT_ENOUGH_RANDOM if the number of random bytes was too small. */
    std::size_t new_outbound_session(
        Account const & local_account,
        _olm_curve25519_public_key const & identity_key,
        _olm_ed25519_public_key const & signing_key,
        _olm_curve25519_public_key const & pre_key,
        std::uint8_t const * prekey_signature,
        _olm_curve25519_public_key const & one_time_key,
        std::uint8_t const * random, std::size_t random_length
    );

    /** Start a new inbound session from a pre-key message.
     * Returns std::size_t(-1) on failure. On failure last_error will be set
     * with an error code. The last_error will be BAD_MESSAGE_FORMAT if
     * the message headers could not be decoded. */
    std::size_t new_inbound_session(
        Account & local_account,
        _olm_curve25519_public_key const * their_identity_key,
        std::uint8_t const * pre_key_message, std::size_t message_length
    );

    /** The number of bytes written by session_id() */
    std::size_t session_id_length() const;

    /** An identifier for this session. Generated by hashing the public keys
     * used to create the session. Returns the length of the session id on
     * success or std::size_t(-1) on failure. On failure last_error will be set
     * with an error code. The last_error will be OUTPUT_BUFFER_TOO_SMALL if
     * the id buffer was too small. */
    std::size_t session_id(
        std::uint8_t * id, std::size_t id_length
    );

    /** True if this session can be used to decode an inbound pre-key message.
     * This can be used to test whether a pre-key message should be decoded
     * with an existing session or if a new session will need to be created.
     * Returns true if the session is the same. Returns false if either the
     * session does not match or the pre-key message could not be decoded.
     */
    bool matches_inbound_session(
        _olm_curve25519_public_key const * their_identity_key,
        std::uint8_t const * pre_key_message, std::size_t message_length
    ) const;

    /** Whether the next message will be a pre-key message or a normal message.
     * An outbound session will send pre-key messages until it receives a
     * message with a ratchet key. */
    MessageType encrypt_message_type() const;

    std::size_t encrypt_message_length(
        std::size_t plaintext_length
    ) const;

    /** The number of bytes of random data the encrypt method will need to
     * encrypt a message. This will be 32 bytes if the session needs to
     * generate a new ephemeral key, or will be 0 bytes otherwise. */
    std::size_t encrypt_random_length() const;

    /** Encrypt some plain-text. Returns the length of the encrypted message
      * or std::size_t(-1) on failure. On failure last_error will be set with
      * an error code. The last_error will be NOT_ENOUGH_RANDOM if the number
      * of random bytes is too small. The last_error will be
      * OUTPUT_BUFFER_TOO_SMALL if the output buffer is too small. */
    std::size_t encrypt(
        std::uint8_t const * plaintext, std::size_t plaintext_length,
        std::uint8_t const * random, std::size_t random_length,
        std::uint8_t * message, std::size_t message_length
    );

    /** An upper bound on the number of bytes of plain-text the decrypt method
     * will write for a given input message length. */
    std::size_t decrypt_max_plaintext_length(
        MessageType message_type,
        std::uint8_t const * message, std::size_t message_length
    );

    /** Decrypt a message. Returns the length of the decrypted plain-text or
     * std::size_t(-1) on failure. On failure last_error will be set with an
     * error code. The last_error will be OUTPUT_BUFFER_TOO_SMALL if the
     * plain-text buffer is too small. The last_error will be
     * BAD_MESSAGE_VERSION if the message was encrypted with an unsupported
     * version of the protocol. The last_error will be BAD_MESSAGE_FORMAT if
     * the message headers could not be decoded. The last_error will be
     * BAD_MESSAGE_MAC if the message could not be verified */
    std::size_t decrypt(
        MessageType message_type,
        std::uint8_t const * message, std::size_t message_length,
        std::uint8_t * plaintext, std::size_t max_plaintext_length
    );

    /**
     * Write a string describing this session and its state (not including the
     * private key) into the buffer provided.
     *
     * Takes a buffer to write to and the length of that buffer
     */
    void describe(char *buf, size_t buflen);
};


std::size_t pickle_length(
    Session const & value
);


std::uint8_t * pickle(
    std::uint8_t * pos,
    Session const & value
);


OLM_EXPORT std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    Session & value
);


} // namespace olm

#endif /* OLM_SESSION_HH_ */
