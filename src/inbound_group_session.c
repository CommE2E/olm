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

#include "olm/inbound_group_session.h"

#include <string.h>

#include "olm/base64.h"
#include "olm/cipher.h"
#include "olm/error.h"
#include "olm/megolm.h"
#include "olm/message.h"

#define OLM_PROTOCOL_VERSION     3

struct OlmInboundGroupSession {
    /** our earliest known ratchet value */
    Megolm initial_ratchet;

    /** The most recent ratchet value */
    Megolm latest_ratchet;

    enum OlmErrorCode last_error;
};

size_t olm_inbound_group_session_size() {
    return sizeof(OlmInboundGroupSession);
}

OlmInboundGroupSession * olm_inbound_group_session(
    void *memory
) {
    OlmInboundGroupSession *session = memory;
    olm_clear_inbound_group_session(session);
    return session;
}

const char *olm_inbound_group_session_last_error(
    const OlmInboundGroupSession *session
) {
    return _olm_error_to_string(session->last_error);
}

size_t olm_clear_inbound_group_session(
    OlmInboundGroupSession *session
) {
    memset(session, 0, sizeof(OlmInboundGroupSession));
    return sizeof(OlmInboundGroupSession);
}

size_t olm_init_inbound_group_session(
    OlmInboundGroupSession *session,
    uint32_t message_index,
    const uint8_t * session_key, size_t session_key_length
) {
    uint8_t key_buf[MEGOLM_RATCHET_LENGTH];
    size_t raw_length = _olm_decode_base64_length(session_key_length);

    if (raw_length == (size_t)-1) {
        session->last_error = OLM_INVALID_BASE64;
        return (size_t)-1;
    }

    if (raw_length != MEGOLM_RATCHET_LENGTH) {
        session->last_error = OLM_BAD_RATCHET_KEY;
        return (size_t)-1;
    }

    _olm_decode_base64(session_key, session_key_length, key_buf);
    megolm_init(&session->initial_ratchet, key_buf, message_index);
    megolm_init(&session->latest_ratchet, key_buf, message_index);
    memset(key_buf, 0, MEGOLM_RATCHET_LENGTH);

    return 0;
}

size_t olm_group_decrypt_max_plaintext_length(
    OlmInboundGroupSession *session,
    uint8_t * message, size_t message_length
) {
    size_t r;
    const struct _olm_cipher *cipher = megolm_cipher();
    struct _OlmDecodeGroupMessageResults decoded_results;

    r = _olm_decode_base64(message, message_length, message);
    if (r == (size_t)-1) {
        session->last_error = OLM_INVALID_BASE64;
        return r;
    }

    _olm_decode_group_message(
        message, message_length,
        cipher->ops->mac_length(cipher),
        &decoded_results);

    if (decoded_results.version != OLM_PROTOCOL_VERSION) {
        session->last_error = OLM_BAD_MESSAGE_VERSION;
        return (size_t)-1;
    }

    if (!decoded_results.ciphertext) {
        session->last_error = OLM_BAD_MESSAGE_FORMAT;
        return (size_t)-1;
    }

    return cipher->ops->decrypt_max_plaintext_length(
        cipher, decoded_results.ciphertext_length);
}


size_t olm_group_decrypt(
    OlmInboundGroupSession *session,
    uint8_t * message, size_t message_length,
    uint8_t * plaintext, size_t max_plaintext_length
) {
    struct _OlmDecodeGroupMessageResults decoded_results;
    const struct _olm_cipher *cipher = megolm_cipher();
    size_t max_length, raw_message_length, r;
    Megolm *megolm;
    Megolm tmp_megolm;

    raw_message_length = _olm_decode_base64(message, message_length, message);
    if (raw_message_length == (size_t)-1) {
        session->last_error = OLM_INVALID_BASE64;
        return (size_t)-1;
    }

    _olm_decode_group_message(
        message, raw_message_length,
        cipher->ops->mac_length(cipher),
        &decoded_results);

    if (decoded_results.version != OLM_PROTOCOL_VERSION) {
        session->last_error = OLM_BAD_MESSAGE_VERSION;
        return (size_t)-1;
    }

    if (!decoded_results.has_chain_index || !decoded_results.session_id
        || !decoded_results.ciphertext
    ) {
        session->last_error = OLM_BAD_MESSAGE_FORMAT;
        return (size_t)-1;
    }

    max_length = cipher->ops->decrypt_max_plaintext_length(
        cipher,
        decoded_results.ciphertext_length
    );
    if (max_plaintext_length < max_length) {
        session->last_error = OLM_OUTPUT_BUFFER_TOO_SMALL;
        return (size_t)-1;
    }

    /* pick a megolm instance to use. If we're at or beyond the latest ratchet
     * value, use that */
    if ((int32_t)(decoded_results.chain_index - session->latest_ratchet.counter) >= 0) {
        megolm = &session->latest_ratchet;
    } else if ((int32_t)(decoded_results.chain_index - session->initial_ratchet.counter) < 0) {
        /* the counter is before our intial ratchet - we can't decode this. */
        session->last_error = OLM_BAD_CHAIN_INDEX;
        return (size_t)-1;
    } else {
        /* otherwise, start from the initial megolm. Take a copy so that we
         * don't overwrite the initial megolm */
        tmp_megolm = session->initial_ratchet;
        megolm = &tmp_megolm;
    }

    megolm_advance_to(megolm, decoded_results.chain_index);

    /* now try checking the mac, and decrypting */
    r = cipher->ops->decrypt(
        cipher,
        megolm_get_data(megolm), MEGOLM_RATCHET_LENGTH,
        message, raw_message_length,
        decoded_results.ciphertext, decoded_results.ciphertext_length,
        plaintext, max_plaintext_length
    );

    memset(&tmp_megolm, 0, sizeof(tmp_megolm));
    if (r == (size_t)-1) {
        session->last_error = OLM_BAD_MESSAGE_MAC;
        return r;
    }

    return r;
}
