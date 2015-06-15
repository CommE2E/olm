#ifndef AXOLOTL_HH_
#define AXOLOTL_HH_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

static const size_t AXOLOTL_MESSAGE_TYPE_PRE_KEY = 0;
static const size_t AXOLOTL_MESSAGE_TYPE_MESSAGE = 1;

struct AxolotlAccount;
struct AxolotlSession;

size_t axolotl_account_size();

size_t axolotl_session_size();

AxolotlAccount * axolotl_account(
    void * memory
);

AxolotlSession * axolotl_session(
    void * memory
);

size_t axolotl_error();

const char * axolotl_account_last_error(
    AxolotlSession * account
);

const char * axolotl_session_last_error(
    AxolotlSession * session
);

size_t axolotl_pickle_account_length(
    AxolotlAccount * account
);

size_t axolotl_pickle_session_length(
    AxolotlSession * session
);

size_t axolotl_pickle_account(
    AxolotlAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

size_t axolotl_pickle_session(
    AxolotlSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

size_t axolotl_unpickle_account(
    AxolotlAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

size_t axolotl_unpickle_session(
    AxolotlSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

size_t axolotl_create_account_random_length(
    AxolotlAccount * account
);

size_t axolotl_create_account(
    AxolotlAccount * account,
    void const * random, size_t random_length
);

size_t axolotl_account_identity_keys_length(
    AxolotlAccount * account
);

size_t axolotl_account_identity_keys(
    AxolotlAccount * account,
    void * identity_keys, size_t identity_key_length
);

size_t axolotl_account_one_time_keys_length(
    AxolotlAccount * account
);

size_t axolotl_account_one_time_keys(
    AxolotlAccount * account,
    void * identity_keys, size_t identity_key_length
);

/* TODO: Add methods for marking keys as used, generating new keys, and
 * tracking which keys have been uploaded to the central servers */

size_t axolotl_create_outbound_session_random_length(
    AxolotlSession * session
);

size_t axolotl_create_outbound_session(
    AxolotlSession * session,
    AxolotlAccount * account,
    void const * their_identity_key, size_t their_identity_key_length,
    unsigned their_one_time_key_id,
    void const * their_one_time_key, size_t their_one_time_key_length,
    void const * random, size_t random_length
);

size_t axolotl_create_inbound_session(
    AxolotlSession * session,
    AxolotlAccount * account,
    void * one_time_key_message, size_t message_length
);

size_t axolotl_matches_inbound_session(
    AxolotlSession * session,
    void * one_time_key_message, size_t message_length
);

size_t axolotl_encrypt_message_type(
    AxolotlSession * session
);

size_t axolotl_encrypt_random_length(
    AxolotlSession * session
);

size_t axolotl_encrypt_message_length(
    AxolotlSession * session,
    size_t plaintext_length
);

size_t axolotl_encrypt(
    AxolotlSession * session,
    void const * plaintext, size_t plaintext_length,
    void const * random, size_t random_length,
    void * message, size_t message_length
);

size_t axolotl_decrypt_max_plaintext_length(
    AxolotlSession * session,
    size_t message_type,
    void * message, size_t message_length
);

size_t axolotl_decrypt(
    AxolotlSession * session,
    size_t message_type,
    void * message, size_t message_length,
    void * plaintext, size_t max_plaintext_length
);



#ifdef __cplusplus
}
#endif

#endif /* AXOLOTL_HH_ */
