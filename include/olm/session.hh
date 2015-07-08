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

namespace olm {

class Account;

enum struct MessageType {
    PRE_KEY = 0,
    MESSAGE = 1,
};

struct Session {

    Session();

    Ratchet ratchet;
    ErrorCode last_error;

    bool received_message;

    Curve25519PublicKey alice_identity_key;
    Curve25519PublicKey alice_base_key;
    Curve25519PublicKey bob_one_time_key;
    std::uint32_t bob_one_time_key_id;

    std::size_t new_outbound_session_random_length();

    std::size_t new_outbound_session(
        Account const & local_account,
        Curve25519PublicKey const & identity_key,
        Curve25519PublicKey const & one_time_key,
        std::uint8_t const * random, std::size_t random_length
    );

    std::size_t new_inbound_session(
        Account & local_account,
        std::uint8_t const * one_time_key_message, std::size_t message_length
    );

    bool matches_inbound_session(
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


std::size_t pickle_length(
    Session const & value
);


std::uint8_t * pickle(
    std::uint8_t * pos,
    Session const & value
);


std::uint8_t const * unpickle(
    std::uint8_t const * pos, std::uint8_t const * end,
    Session & value
);


} // namespace olm

#endif /* OLM_SESSION_HH_ */
