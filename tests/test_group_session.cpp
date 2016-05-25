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
#include "olm/outbound_group_session.h"
#include "unittest.hh"


int main() {

{
    TestCase test_case("Pickle outbound group session");

    size_t size = olm_outbound_group_session_size();
    uint8_t memory[size];
    OlmOutboundGroupSession *session = olm_outbound_group_session(memory);

    size_t pickle_length = olm_pickle_outbound_group_session_length(session);
    uint8_t pickle1[pickle_length];
    olm_pickle_outbound_group_session(session,
                                      "secret_key", 10,
                                      pickle1, pickle_length);
    uint8_t pickle2[pickle_length];
    memcpy(pickle2, pickle1, pickle_length);

    uint8_t buffer2[size];
    OlmOutboundGroupSession *session2 = olm_outbound_group_session(buffer2);
    size_t res = olm_unpickle_outbound_group_session(session2,
                                                     "secret_key", 10,
                                                     pickle2, pickle_length);
    assert_not_equals((size_t)-1, res);
    assert_equals(pickle_length,
                  olm_pickle_outbound_group_session_length(session2));
    olm_pickle_outbound_group_session(session2,
                                      "secret_key", 10,
                                      pickle2, pickle_length);

    assert_equals(pickle1, pickle2, pickle_length);
}


{
    TestCase test_case("Pickle inbound group session");

    size_t size = olm_inbound_group_session_size();
    uint8_t memory[size];
    OlmInboundGroupSession *session = olm_inbound_group_session(memory);

    size_t pickle_length = olm_pickle_inbound_group_session_length(session);
    uint8_t pickle1[pickle_length];
    olm_pickle_inbound_group_session(session,
                                     "secret_key", 10,
                                     pickle1, pickle_length);
    uint8_t pickle2[pickle_length];
    memcpy(pickle2, pickle1, pickle_length);

    uint8_t buffer2[size];
    OlmInboundGroupSession *session2 = olm_inbound_group_session(buffer2);
    size_t res = olm_unpickle_inbound_group_session(session2,
                                                    "secret_key", 10,
                                                    pickle2, pickle_length);
    assert_not_equals((size_t)-1, res);
    assert_equals(pickle_length,
                  olm_pickle_inbound_group_session_length(session2));
    olm_pickle_inbound_group_session(session2,
                                      "secret_key", 10,
                                      pickle2, pickle_length);

    assert_equals(pickle1, pickle2, pickle_length);
}


{
    TestCase test_case("Group message send/receive");

    uint8_t random_bytes[] =
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF";


    /* build the outbound session */
    size_t size = olm_outbound_group_session_size();
    uint8_t memory[size];
    OlmOutboundGroupSession *session = olm_outbound_group_session(memory);

    assert_equals((size_t)132,
                  olm_init_outbound_group_session_random_length(session));

    size_t res = olm_init_outbound_group_session(
        session, random_bytes, sizeof(random_bytes));
    assert_equals((size_t)0, res);

    assert_equals(0U, olm_outbound_group_session_message_index(session));
    size_t session_key_len = olm_outbound_group_session_key_length(session);
    uint8_t session_key[session_key_len];
    olm_outbound_group_session_key(session, session_key, session_key_len);


    /* encode the message */
    uint8_t plaintext[] = "Message";
    size_t plaintext_length = sizeof(plaintext) - 1;

    size_t msglen = olm_group_encrypt_message_length(
        session, plaintext_length);

    uint8_t msg[msglen];
    res = olm_group_encrypt(session, plaintext, plaintext_length,
                            msg, msglen);
    assert_equals(msglen, res);
    assert_equals(1U, olm_outbound_group_session_message_index(session));


    /* build the inbound session */
    size = olm_inbound_group_session_size();
    uint8_t inbound_session_memory[size];
    OlmInboundGroupSession *inbound_session =
        olm_inbound_group_session(inbound_session_memory);

    res = olm_init_inbound_group_session(
        inbound_session, 0U, session_key, session_key_len);
    assert_equals((size_t)0, res);

    /* decode the message */

    /* olm_group_decrypt_max_plaintext_length destroys the input so we have to
       copy it. */
    uint8_t msgcopy[msglen];
    memcpy(msgcopy, msg, msglen);
    size = olm_group_decrypt_max_plaintext_length(inbound_session, msgcopy, msglen);
    uint8_t plaintext_buf[size];
    res = olm_group_decrypt(inbound_session, msg, msglen,
                            plaintext_buf, size);
    assert_equals(plaintext_length, res);
    assert_equals(plaintext, plaintext_buf, res);
}

}
