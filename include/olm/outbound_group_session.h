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
#ifndef OLM_OUTBOUND_GROUP_SESSION_H_
#define OLM_OUTBOUND_GROUP_SESSION_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OlmOutboundGroupSession OlmOutboundGroupSession;

/** get the size of an outbound group session, in bytes. */
size_t olm_outbound_group_session_size();

/**
 * Initialise an outbound group session object using the supplied memory
 * The supplied memory should be at least olm_outbound_group_session_size()
 * bytes.
 */
OlmOutboundGroupSession * olm_outbound_group_session(
    void *memory
);

/**
 * A null terminated string describing the most recent error to happen to a
 * group session */
const char *olm_outbound_group_session_last_error(
    const OlmOutboundGroupSession *session
);

/** Clears the memory used to back this group session */
size_t olm_clear_outbound_group_session(
    OlmOutboundGroupSession *session
);

/** The number of random bytes needed to create an outbound group session */
size_t olm_init_outbound_group_session_random_length(
    const OlmOutboundGroupSession *session
);

/**
 * Start a new outbound group session. Returns std::size_t(-1) on failure. On
 * failure last_error will be set with an error code. The last_error will be
 * NOT_ENOUGH_RANDOM if the number of random bytes was too small.
 */
size_t olm_init_outbound_group_session(
    OlmOutboundGroupSession *session,
    uint8_t const * random, size_t random_length
);

/**
 * The number of bytes that will be created by encrypting a message
 */
size_t olm_group_encrypt_message_length(
    OlmOutboundGroupSession *session,
    size_t plaintext_length
);

/**
 * Encrypt some plain-text. Returns the length of the encrypted message or
 * std::size_t(-1) on failure. On failure last_error will be set with an
 * error code. The last_error will be OUTPUT_BUFFER_TOO_SMALL if the output
 * buffer is too small.
 */
size_t olm_group_encrypt(
    OlmOutboundGroupSession *session,
    uint8_t const * plaintext, size_t plaintext_length,
    uint8_t * message, size_t message_length
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OLM_OUTBOUND_GROUP_SESSION_H_ */
