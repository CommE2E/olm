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
#include "unittest.hh"


int main() {

{

    TestCase test_case("Pickle outbound group");

    size_t size = olm_outbound_group_session_size();
    void *memory = alloca(size);
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
    TestCase test_case("Group message send/receive");

    uint8_t random_bytes[] =
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF"
        "0123456789ABDEF0123456789ABCDEF";



    size_t size = olm_outbound_group_session_size();
    void *memory = alloca(size);
    OlmOutboundGroupSession *session = olm_outbound_group_session(memory);

    assert_equals((size_t)132,
                  olm_init_outbound_group_session_random_length(session));

    size_t res = olm_init_outbound_group_session(
        session, random_bytes, sizeof(random_bytes));
    assert_equals((size_t)0, res);

    uint8_t plaintext[] = "Message";
    size_t plaintext_length = sizeof(plaintext) - 1;

    size_t msglen = olm_group_encrypt_message_length(
        session, plaintext_length);

    uint8_t *msg = (uint8_t *)alloca(msglen);
    res = olm_group_encrypt(session, plaintext, plaintext_length,
                            msg, msglen);
    assert_equals(msglen, res);

    // TODO: decode the message
}

}
