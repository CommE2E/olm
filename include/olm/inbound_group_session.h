/* Copyright 2016 OpenMarket Ltd
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
#ifndef OLM_INBOUND_GROUP_SESSION_H_
#define OLM_INBOUND_GROUP_SESSION_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OlmInboundGroupSession OlmInboundGroupSession;

/** get the size of an inbound group session, in bytes. */
size_t olm_inbound_group_session_size();

/**
 * Initialise an inbound group session object using the supplied memory
 * The supplied memory should be at least olm_inbound_group_session_size()
 * bytes.
 */
OlmInboundGroupSession * olm_inbound_group_session(
    void *memory
);

/**
 * A null terminated string describing the most recent error to happen to a
 * group session */
const char *olm_inbound_group_session_last_error(
    const OlmInboundGroupSession *session
);

/** Clears the memory used to back this group session */
size_t olm_clear_inbound_group_session(
    OlmInboundGroupSession *session
);

/** Returns the number of bytes needed to store an inbound group session */
size_t olm_pickle_inbound_group_session_length(
    const OlmInboundGroupSession *session
);

/**
 * Stores a group session as a base64 string. Encrypts the session using the
 * supplied key. Returns the length of the session on success.
 *
 * Returns olm_error() on failure. If the pickle output buffer
 * is smaller than olm_pickle_inbound_group_session_length() then
 * olm_inbound_group_session_last_error() will be "OUTPUT_BUFFER_TOO_SMALL"
 */
size_t olm_pickle_inbound_group_session(
    OlmInboundGroupSession *session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/**
 * Loads a group session from a pickled base64 string. Decrypts the session
 * using the supplied key.
 *
 * Returns olm_error() on failure. If the key doesn't match the one used to
 * encrypt the account then olm_inbound_group_session_last_error() will be
 * "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded then
 * olm_inbound_group_session_last_error() will be "INVALID_BASE64". The input
 * pickled buffer is destroyed
 */
size_t olm_unpickle_inbound_group_session(
    OlmInboundGroupSession *session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);


/**
 * Start a new inbound group session, based on the parameters supplied.
 *
 * Returns olm_error() on failure. On failure last_error will be set with an
 * error code. The last_error will be:
 *
 *  * OLM_INVALID_BASE64  if the session_key is not valid base64
 *  * OLM_BAD_RATCHET_KEY if the session_key is invalid
 */
size_t olm_init_inbound_group_session(
    OlmInboundGroupSession *session,
    uint32_t message_index,

    /* base64-encoded key */
    uint8_t const * session_key, size_t session_key_length
);

/**
 * Get an upper bound on the number of bytes of plain-text the decrypt method
 * will write for a given input message length. The actual size could be
 * different due to padding.
 *
 * The input message buffer is destroyed.
 *
 * Returns olm_error() on failure.
 */
size_t olm_group_decrypt_max_plaintext_length(
    OlmInboundGroupSession *session,
    uint8_t * message, size_t message_length
);

/**
 * Decrypt a message.
 *
 * The input message buffer is destroyed.
 *
 * Returns the length of the decrypted plain-text, or olm_error() on failure.
 *
 * On failure last_error will be set with an error code. The last_error will
 * be:
 *   * OLM_OUTPUT_BUFFER_TOO_SMALL if the plain-text buffer is too small
 *   * OLM_INVALID_BASE64 if the message is not valid base-64
 *   * OLM_BAD_MESSAGE_VERSION if the message was encrypted with an unsupported
 *     version of the protocol
 *   * OLM_BAD_MESSAGE_FORMAT if the message headers could not be decoded
 *   * OLM_BAD_MESSAGE_MAC if the message could not be verified
 *   * OLM_BAD_CHAIN_INDEX if we do not have a ratchet key corresponding to the
 *     message's index (ie, it was sent before the ratchet key was shared with
 *     us)
 */
size_t olm_group_decrypt(
    OlmInboundGroupSession *session,

    /* input; note that it will be overwritten with the base64-decoded
       message. */
    uint8_t * message, size_t message_length,

    /* output */
    uint8_t * plaintext, size_t max_plaintext_length
);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OLM_INBOUND_GROUP_SESSION_H_ */
