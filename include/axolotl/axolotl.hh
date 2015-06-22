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

/** The size of an account object in bytes */
size_t axolotl_account_size();

/** The size of a session object in bytes */
size_t axolotl_session_size();

/** Initialise an account object using the supplied memory
 *  The supplied memory must be at least axolotl_account_size() bytes */
AxolotlAccount * axolotl_account(
    void * memory
);

/** Initialise a session object using the supplied memory
 *  The supplied memory must be at least axolotl_session_size() bytes */
AxolotlSession * axolotl_session(
    void * memory
);

/** The value that axolotl will return from a function if there was an error */
size_t axolotl_error();

/** A null terminated string describing the most recent error to happen to an
 * account */
const char * axolotl_account_last_error(
    AxolotlSession * account
);

/** A null terminated string describing the most recent error to happen to a
 * session */
const char * axolotl_session_last_error(
    AxolotlSession * session
);

/** Returns the number of bytes needed to store an account */
size_t axolotl_pickle_account_length(
    AxolotlAccount * account
);

/** Returns the number of bytes needed to store a session */
size_t axolotl_pickle_session_length(
    AxolotlSession * session
);

/** Stores an account as a base64 string. Encrypts the account using the
 * supplied key. Returns the length of the pickled account on success.
 * Returns axolotl_error() on failure. If the pickle output buffer
 * is smaller than axolotl_pickle_account_length() then
 * axolotl_account_last_error() will be "OUTPUT_BUFFER_TOO_SMALL" */
size_t axolotl_pickle_account(
    AxolotlAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** Stores a session as a base64 string. Encrypts the session using the
 * supplied key. Returns the length of the pickled session on success.
 * Returns axolotl_error() on failure. If the pickle output buffer
 * is smaller than axolotl_pickle_session_length() then
 * axolotl_session_last_error() will be "OUTPUT_BUFFER_TOO_SMALL" */
size_t axolotl_pickle_session(
    AxolotlSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** Loads an account from a pickled base64 string. Decrypts the account using
 * the supplied key. Returns axolotl_error() on failure. If the key doesn't
 * match the one used to encrypt the account then axolotl_account_last_error()
 * will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded then
 * axolotl_account_last_error() will be "INVALID_BASE64". */
size_t axolotl_unpickle_account(
    AxolotlAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** Loads a session from a pickled base64 string. Decrypts the session using
 * the supplied key. Returns axolotl_error() on failure. If the key doesn't
 * match the one used to encrypt the account then axolotl_session_last_error()
 * will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded then
 * axolotl_session_last_error() will be "INVALID_BASE64". */
size_t axolotl_unpickle_session(
    AxolotlSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** The number of random bytes needed to create an account.*/
size_t axolotl_create_account_random_length(
    AxolotlAccount * account
);

/** Creates a new account. Returns axolotl_error() on failure. If weren't
 * enough random bytes then axolotl_account_last_error() will be
 * "NOT_ENOUGH_RANDOM" */
size_t axolotl_create_account(
    AxolotlAccount * account,
    void const * random, size_t random_length
);

/** The size of the output buffer needed to hold the identity keys */
size_t axolotl_account_identity_keys_length(
    AxolotlAccount * account
);

/** Writes the public parts of the identity keys for the account into the
 * identity_keys output buffer. The output is formatted as though it was
 * created with sprintf(output, "[[%10d,\"%43s\"]\n]", key_id, key_base64).
 * The output can either be parsed as fixed width using the above format or by
 * a JSON parser. Returns axolotl_error() on failure. If the identity_keys
 * buffer was too small then axolotl_account_last_error() will be
 * "OUTPUT_BUFFER_TOO_SMALL". */
size_t axolotl_account_identity_keys(
    AxolotlAccount * account,
    void * identity_keys, size_t identity_key_length
);

/** The size of the output buffer needed to hold the one time keys */
size_t axolotl_account_one_time_keys_length(
    AxolotlAccount * account
);

/** Writes the public parts of the one time keys for the account into the
 * one_time_keys output buffer. The first key will be formatted as though it was
 * created with sprintf(output, "[[%10d,\"%43s\"]\n", key_id, key_base64).
 * subsequent keys are formatted with ",[%10d,\"%43s\"]\n". The final byte of
 * output will be "]". The output can either be parsed as fixed width using
 * the above format or by a JSON parser. Returns axolotl_error() on failure.
 * If the one_time_keys buffer was too small then axolotl_account_last_error()
 * will be "OUTPUT_BUFFER_TOO_SMALL". */
size_t axolotl_account_one_time_keys(
    AxolotlAccount * account,
    void * one_time_keys, size_t one_time_keys_length
);

/* TODO: Add methods for marking keys as used, generating new keys, and
 * tracking which keys have been uploaded to the central servers */

/** The number of random bytes needed to create an outbound session */
size_t axolotl_create_outbound_session_random_length(
    AxolotlSession * session
);

/** Creates a new out-bound session for sending messages to a given identity_key
 * and one_time_key. Returns axolotl_error() on failure. If the keys couldn't be
 * decoded as base64 then axolotl_session_last_error() will be "INVALID_BASE64"
 * If there weren't enough random bytes then axolotl_session_last_error() will
 * be "NOT_ENOUGH_RANDOM". */
size_t axolotl_create_outbound_session(
    AxolotlSession * session,
    AxolotlAccount * account,
    void const * their_identity_key, size_t their_identity_key_length,
    unsigned their_one_time_key_id,
    void const * their_one_time_key, size_t their_one_time_key_length,
    void const * random, size_t random_length
);

/** Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message. Returns axolotl_error() on failure. If the base64
 * couldn't be decoded then axolotl_session_last_error will be "INVALID_BASE64".
 * If the message was for an unsupported protocol version then
 * axolotl_session_last_error() will be "BAD_MESSAGE_VERSION". If the message
 * couldn't be decoded then then axolotl_session_last_error() will be
 * "BAD_MESSAGE_FORMAT". If the message refers to an unknown one time
 * key then axolotl_session_last_error() will be "BAD_MESSAGE_KEY_ID". */
size_t axolotl_create_inbound_session(
    AxolotlSession * session,
    AxolotlAccount * account,
    void * one_time_key_message, size_t message_length
);

/** Checks if the PRE_KEY message is for this in-bound session. This can happen
 * if multiple messages are sent to this account before this account sends a
 * message in reply. Returns axolotl_error() on failure. If the base64
 * couldn't be decoded then axolotl_session_last_error will be "INVALID_BASE64".
 * If the message was for an unsupported protocol version then
 * axolotl_session_last_error() will be "BAD_MESSAGE_VERSION". If the message
 * couldn't be decoded then then axolotl_session_last_error() will be
 * "BAD_MESSAGE_FORMAT". */
size_t axolotl_matches_inbound_session(
    AxolotlSession * session,
    void * one_time_key_message, size_t message_length
);

/** Removes the one time keys that the session used from the account. Returns
 * axolotl_error() on failure. If the account doesn't have any matching one time
 * keys then axolotl_account_last_error() will be "BAD_MESSAGE_KEY_ID". */
size_t axolotl_remove_one_time_keys(
    AxolotlAccount * account,
    AxolotlSession * session
);

/** The type of the next message that axolotl_encrypt() will return. Returns
 * AXOLOTL_MESSAGE_TYPE_PRE_KEY if the message will be a PRE_KEY message.
 * Returns AXOLOTL_MESSAGE_TYPE_MESSAGE if the message will be a normal message.
 * Returns axolotl_error on failure. */
size_t axolotl_encrypt_message_type(
    AxolotlSession * session
);

/** The number of random bytes needed to encrypt the next message. */
size_t axolotl_encrypt_random_length(
    AxolotlSession * session
);

/** The size of the next message in bytes for the given number of plain-text
 * bytes. */
size_t axolotl_encrypt_message_length(
    AxolotlSession * session,
    size_t plaintext_length
);

/** Encrypts a message using the session. Returns the length of the message in
 * bytes on success. Writes the message as base64 into the message buffer.
 * Returns axolotl_error() on failure. If the message buffer is too small then
 * axolotl_session_last_error() will be "OUTPUT_BUFFER_TOO_SMALL". If there
 * weren't enough random bytes then axolotl_session_last_error() will be
 * "NOT_ENOUGH_RANDOM". */
size_t axolotl_encrypt(
    AxolotlSession * session,
    void const * plaintext, size_t plaintext_length,
    void const * random, size_t random_length,
    void * message, size_t message_length
);

/** The maximum number of bytes of plain-text a given message could decode to.
 * The actual size could be different due to padding. Returns axolotl_error()
 * on failures. If the message base64 couldn't be decoded then
 * axolotl_session_last_error() will be "INVALID_BASE64". If the message
 * is for an unsupported version of the protocol then
 * axolotl_session_last_error() will be "BAD_MESSAGE_VERSION". If the message
 * couldn't be decoded then axolotl_session_last_error() will be
 * "BAD_MESSAGE_FORMAT". */
size_t axolotl_decrypt_max_plaintext_length(
    AxolotlSession * session,
    size_t message_type,
    void * message, size_t message_length
);

/** Decrypts a message using the session. Returns the length of the plain-text
 * on success. Returns axolotl_error() on failure. If the plain-text buffer
 * is smaller than axolotl_decrypt_max_plaintext_length() then
 * axolotl_session_last_error() will be "OUTPUT_BUFFER_TOO_SMALL". If the
 * base64 couldn't be decoded then axolotl_session_last_error() will be
 * "INVALID_BASE64". If the message is for an unsupported version of the
 * protocol then axolotl_session_last_error() will be "BAD_MESSAGE_VERSION".
 * If the message couldn't be decoded then axolotl_session_last_error() will be
 * "BAD_MESSAGE_FORMAT". If the MAC on the message was invalid then
 * axolotl_session_last_error() will be "BAD_MESSAGE_MAC". */
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
