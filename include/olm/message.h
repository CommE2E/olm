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

/**
 * functions for encoding and decoding messages in the Olm protocol.
 *
 * Some of these functions have only C++ bindings, and are declared in
 * message.hh; in time, they should probably be converted to plain C and
 * declared here.
 */

#ifndef OLM_MESSAGE_H_
#define OLM_MESSAGE_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The length of the buffer needed to hold a group message.
 */
size_t _olm_encode_group_message_length(
    size_t group_session_id_length,
    uint32_t chain_index,
    size_t ciphertext_length,
    size_t mac_length
);

/**
 * Writes the message headers into the output buffer.
 *
 * version:            version number of the olm protocol
 * session_id:         group session identifier
 * session_id_length:  length of session_id
 * chain_index:        message index
 * ciphertext_length:  length of the ciphertext
 * output:             where to write the output. Should be at least
 *                     olm_encode_group_message_length() bytes long.
 * ciphertext_ptr:     returns the address that the ciphertext
 *                     should be written to, followed by the MAC.
 */
void _olm_encode_group_message(
    uint8_t version,
    const uint8_t *session_id,
    size_t session_id_length,
    uint32_t chain_index,
    size_t ciphertext_length,
    uint8_t *output,
    uint8_t **ciphertext_ptr
);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OLM_MESSAGE_H_ */
