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

#include "olm/outbound_group_session.h"

#include <string.h>
#include <sys/time.h>

#include "olm/base64.h"
#include "olm/cipher.h"
#include "olm/error.h"
#include "olm/megolm.h"
#include "olm/message.h"

#define OLM_PROTOCOL_VERSION     3
#define SESSION_ID_RANDOM_BYTES  4
#define GROUP_SESSION_ID_LENGTH (sizeof(struct timeval) + SESSION_ID_RANDOM_BYTES)

struct OlmOutboundGroupSession {
    /** the Megolm ratchet providing the encryption keys */
    Megolm ratchet;

    /** unique identifier for this session */
    uint8_t session_id[GROUP_SESSION_ID_LENGTH];

    enum OlmErrorCode last_error;
};


size_t olm_outbound_group_session_size() {
    return sizeof(OlmOutboundGroupSession);
}

OlmOutboundGroupSession * olm_outbound_group_session(
    void *memory
) {
    OlmOutboundGroupSession *session = memory;
    olm_clear_outbound_group_session(session);
    return session;
}

const char *olm_outbound_group_session_last_error(
    const OlmOutboundGroupSession *session
) {
    return _olm_error_to_string(session->last_error);
}

size_t olm_clear_outbound_group_session(
    OlmOutboundGroupSession *session
) {
    memset(session, 0, sizeof(OlmOutboundGroupSession));
    return sizeof(OlmOutboundGroupSession);
}

size_t olm_init_outbound_group_session_random_length(
    const OlmOutboundGroupSession *session
) {
    /* we need data to initialize the megolm ratchet, plus some more for the
     * session id.
     */
    return MEGOLM_RATCHET_LENGTH + SESSION_ID_RANDOM_BYTES;
}

size_t olm_init_outbound_group_session(
    OlmOutboundGroupSession *session,
    uint8_t const * random, size_t random_length
) {
    if (random_length < olm_init_outbound_group_session_random_length(session)) {
        /* Insufficient random data for new session */
        session->last_error = OLM_NOT_ENOUGH_RANDOM;
        return (size_t)-1;
    }

    megolm_init(&(session->ratchet), random, 0);
    random += MEGOLM_RATCHET_LENGTH;

    /* initialise the session id. This just has to be unique. We use the
     * current time plus some random data.
     */
    gettimeofday((struct timeval *)(session->session_id), NULL);
    memcpy((session->session_id) + sizeof(struct timeval),
           random, SESSION_ID_RANDOM_BYTES);

    return 0;
}

static size_t raw_message_length(
    OlmOutboundGroupSession *session,
    size_t plaintext_length)
{
    size_t ciphertext_length, mac_length;
    const struct _olm_cipher *cipher = megolm_cipher();

    ciphertext_length = cipher->ops->encrypt_ciphertext_length(
        cipher, plaintext_length
    );

    mac_length = cipher->ops->mac_length(cipher);

    return _olm_encode_group_message_length(
        GROUP_SESSION_ID_LENGTH, session->ratchet.counter,
        ciphertext_length, mac_length);
}

size_t olm_group_encrypt_message_length(
    OlmOutboundGroupSession *session,
    size_t plaintext_length
) {
    size_t message_length = raw_message_length(session, plaintext_length);
    return _olm_encode_base64_length(message_length);
}


size_t olm_group_encrypt(
    OlmOutboundGroupSession *session,
    uint8_t const * plaintext, size_t plaintext_length,
    uint8_t * message, size_t max_message_length
) {
    size_t ciphertext_length;
    size_t rawmsglen;
    size_t result;
    uint8_t *ciphertext_ptr, *message_pos;
    const struct _olm_cipher *cipher = megolm_cipher();

    rawmsglen = raw_message_length(session, plaintext_length);

    if (max_message_length < _olm_encode_base64_length(rawmsglen)) {
        session->last_error = OLM_OUTPUT_BUFFER_TOO_SMALL;
        return (size_t)-1;
    }

    ciphertext_length = cipher->ops->encrypt_ciphertext_length(
        cipher,
        plaintext_length
    );

    /* we construct the message at the end of the buffer, so that
     * we have room to base64-encode it once we're done.
     */
    message_pos = message + _olm_encode_base64_length(rawmsglen) - rawmsglen;

    /* first we build the message structure, then we encrypt
     * the plaintext into it.
     */
    _olm_encode_group_message(
        OLM_PROTOCOL_VERSION,
        session->session_id, GROUP_SESSION_ID_LENGTH,
        session->ratchet.counter,
        ciphertext_length,
        message_pos,
        &ciphertext_ptr);

    result = cipher->ops->encrypt(
        cipher,
        megolm_get_data(&(session->ratchet)), MEGOLM_RATCHET_LENGTH,
        plaintext, plaintext_length,
        ciphertext_ptr, ciphertext_length,
        message_pos, rawmsglen
    );

    if (result == (size_t)-1) {
        return result;
    }

    megolm_advance(&(session->ratchet));

    return _olm_encode_base64(
        message_pos, rawmsglen,
        message
    );
}
