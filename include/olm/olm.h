/* Copyright 2015, 2016 OpenMarket Ltd
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

#ifndef OLM_H_
#define OLM_H_

#include <stddef.h>
#include <stdint.h>

#include "olm/error.h"
#include "olm/inbound_group_session.h"
#include "olm/outbound_group_session.h"

#include "olm/olm_export.h"

#ifdef __cplusplus
extern "C" {
#endif

static const size_t OLM_MESSAGE_TYPE_PRE_KEY = 0;
static const size_t OLM_MESSAGE_TYPE_MESSAGE = 1;

typedef struct OlmAccount OlmAccount;
typedef struct OlmSession OlmSession;
typedef struct OlmUtility OlmUtility;

/** Get the version number of the library.
 * Arguments will be updated if non-null.
 */
OLM_EXPORT void olm_get_library_version(uint8_t *major, uint8_t *minor, uint8_t *patch);

/** The size of an account object in bytes */
OLM_EXPORT size_t olm_account_size(void);

/** The size of a session object in bytes */
OLM_EXPORT size_t olm_session_size(void);

/** The size of a utility object in bytes */
OLM_EXPORT size_t olm_utility_size(void);

/** Initialise an account object using the supplied memory
 *  The supplied memory must be at least olm_account_size() bytes */
OLM_EXPORT OlmAccount * olm_account(
    void * memory
);

/** Initialise a session object using the supplied memory
 *  The supplied memory must be at least olm_session_size() bytes */
OLM_EXPORT OlmSession * olm_session(
    void * memory
);

/** Initialise a utility object using the supplied memory
 *  The supplied memory must be at least olm_utility_size() bytes */
OLM_EXPORT OlmUtility * olm_utility(
    void * memory
);

/** The value that olm will return from a function if there was an error */
OLM_EXPORT size_t olm_error(void);

/** A null terminated string describing the most recent error to happen to an
 * account */
OLM_EXPORT const char * olm_account_last_error(
    OlmAccount const * account
);

/** An error code describing the most recent error to happen to an account */
OLM_EXPORT enum OlmErrorCode olm_account_last_error_code(
    OlmAccount const * account
);

/** A null terminated string describing the most recent error to happen to a
 * session */
OLM_EXPORT const char * olm_session_last_error(
    OlmSession const * session
);

/** An error code describing the most recent error to happen to a session */
OLM_EXPORT enum OlmErrorCode olm_session_last_error_code(
    OlmSession const * session
);

/** A null terminated string describing the most recent error to happen to a
 * utility */
OLM_EXPORT const char * olm_utility_last_error(
    OlmUtility const * utility
);

/** An error code describing the most recent error to happen to a utility */
OLM_EXPORT enum OlmErrorCode olm_utility_last_error_code(
    OlmUtility const * utility
);

/** Clears the memory used to back this account */
OLM_EXPORT size_t olm_clear_account(
    OlmAccount * account
);

/** Clears the memory used to back this session */
OLM_EXPORT size_t olm_clear_session(
    OlmSession * session
);

/** Clears the memory used to back this utility */
OLM_EXPORT size_t olm_clear_utility(
    OlmUtility * utility
);

/** Returns the number of bytes needed to store an account */
OLM_EXPORT size_t olm_pickle_account_length(
    OlmAccount const * account
);

/** Returns the number of bytes needed to store a session */
OLM_EXPORT size_t olm_pickle_session_length(
    OlmSession const * session
);

/** Stores an account as a base64 string. Encrypts the account using the
 * supplied key. Returns the length of the pickled account on success.
 * Returns olm_error() on failure. If the pickle output buffer
 * is smaller than olm_pickle_account_length() then
 * olm_account_last_error() will be "OUTPUT_BUFFER_TOO_SMALL" */
OLM_EXPORT size_t olm_pickle_account(
    OlmAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** Stores a session as a base64 string. Encrypts the session using the
 * supplied key. Returns the length of the pickled session on success.
 * Returns olm_error() on failure. If the pickle output buffer
 * is smaller than olm_pickle_session_length() then
 * olm_session_last_error() will be "OUTPUT_BUFFER_TOO_SMALL" */
OLM_EXPORT size_t olm_pickle_session(
    OlmSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** Loads an account from a pickled base64 string. Decrypts the account using
 * the supplied key. Returns olm_error() on failure. If the key doesn't
 * match the one used to encrypt the account then olm_account_last_error()
 * will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded then
 * olm_account_last_error() will be "INVALID_BASE64". The input pickled
 * buffer is destroyed */
OLM_EXPORT size_t olm_unpickle_account(
    OlmAccount * account,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** Loads a session from a pickled base64 string. Decrypts the session using
 * the supplied key. Returns olm_error() on failure. If the key doesn't
 * match the one used to encrypt the account then olm_session_last_error()
 * will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded then
 * olm_session_last_error() will be "INVALID_BASE64". The input pickled
 * buffer is destroyed */
OLM_EXPORT size_t olm_unpickle_session(
    OlmSession * session,
    void const * key, size_t key_length,
    void * pickled, size_t pickled_length
);

/** The number of random bytes needed to create an account.*/
OLM_EXPORT size_t olm_create_account_random_length(
    OlmAccount const * account
);

/** Creates a new account. Returns olm_error() on failure. If there weren't
 * enough random bytes then olm_account_last_error() will be
 * "NOT_ENOUGH_RANDOM" */
OLM_EXPORT size_t olm_create_account(
    OlmAccount * account,
    void * random, size_t random_length
);

/** The size of the output buffer needed to hold the identity keys */
OLM_EXPORT size_t olm_account_identity_keys_length(
    OlmAccount const * account
);

/** Writes the public parts of the identity keys for the account into the
 * identity_keys output buffer. Returns olm_error() on failure. If the
 * identity_keys buffer was too small then olm_account_last_error() will be
 * "OUTPUT_BUFFER_TOO_SMALL". */
OLM_EXPORT size_t olm_account_identity_keys(
    OlmAccount * account,
    void * identity_keys, size_t identity_key_length
);


/** The length of an ed25519 signature encoded as base64. */
OLM_EXPORT size_t olm_account_signature_length(
    OlmAccount const * account
);

/** Signs a message with the ed25519 key for this account. Returns olm_error()
 * on failure. If the signature buffer was too small then
 * olm_account_last_error() will be "OUTPUT_BUFFER_TOO_SMALL" */
OLM_EXPORT size_t olm_account_sign(
    OlmAccount * account,
    void const * message, size_t message_length,
    void * signature, size_t signature_length
);

/** The size of the output buffer needed to hold the one time keys */
OLM_EXPORT size_t olm_account_one_time_keys_length(
    OlmAccount const * account
);

/** Writes the public parts of the unpublished one time keys for the account
 * into the one_time_keys output buffer.
 * <p>
 * The returned data is a JSON-formatted object with the single property
 * <tt>curve25519</tt>, which is itself an object mapping key id to
 * base64-encoded Curve25519 key. For example:
 * <pre>
 * {
 *     curve25519: {
 *         "AAAAAA": "wo76WcYtb0Vk/pBOdmduiGJ0wIEjW4IBMbbQn7aSnTo",
 *         "AAAAAB": "LRvjo46L1X2vx69sS9QNFD29HWulxrmW11Up5AfAjgU"
 *     }
 * }
 * </pre>
 * Returns olm_error() on failure.
 * <p>
 * If the one_time_keys buffer was too small then olm_account_last_error()
 * will be "OUTPUT_BUFFER_TOO_SMALL". */
OLM_EXPORT size_t olm_account_one_time_keys(
    OlmAccount * account,
    void * one_time_keys, size_t one_time_keys_length
);

/** Marks the current set of one time keys and fallback key as being published
 * Once marked as published, the one time keys will no longer be returned by
 * olm_account_one_time_keys(), and the fallback key will no longer be returned
 * by olm_account_unpublished_fallback_key().
 *
 * Returns the number of one-time keys that were marked as published.  Note that
 * this count does not include the fallback key. */
OLM_EXPORT size_t olm_account_mark_keys_as_published(
    OlmAccount * account
);

/** The largest number of one time keys this account can store. */
OLM_EXPORT size_t olm_account_max_number_of_one_time_keys(
    OlmAccount const * account
);

/** The number of random bytes needed to generate a given number of new one
 * time keys. */
OLM_EXPORT size_t olm_account_generate_one_time_keys_random_length(
    OlmAccount const * account,
    size_t number_of_keys
);

/** Generates a number of new one time keys. If the total number of keys stored
 * by this account exceeds max_number_of_one_time_keys() then the old keys are
 * discarded. Returns olm_error() on error. If the number of random bytes is
 * too small then olm_account_last_error() will be "NOT_ENOUGH_RANDOM". */
OLM_EXPORT size_t olm_account_generate_one_time_keys(
    OlmAccount * account,
    size_t number_of_keys,
    void * random, size_t random_length
);

OLM_EXPORT size_t olm_account_generate_prekey_random_length(
    OlmAccount const * account
);

OLM_EXPORT size_t olm_account_generate_prekey(
    OlmAccount * account,
    void * random, size_t random_length
);

OLM_EXPORT size_t olm_account_prekey_length(
    OlmAccount const * account
);

OLM_EXPORT size_t olm_account_prekey(
    OlmAccount * account,
    void * prekey_json, size_t prekey_json_length
);

OLM_EXPORT void olm_account_forget_old_prekey(
    OlmAccount * account
);

OLM_EXPORT void olm_account_mark_prekey_as_published(
    OlmAccount * account
);

OLM_EXPORT uint64_t olm_account_get_last_prekey_publish_time(
    OlmAccount * account
);

OLM_EXPORT size_t olm_account_unpublished_prekey(
    OlmAccount * account,
    void * prekey_json, size_t prekey_json_length
);

OLM_EXPORT size_t olm_account_prekey_signature(
    OlmAccount * account,
    void * signature
);

/** The number of random bytes needed to generate a fallback key. */
OLM_EXPORT size_t olm_account_generate_fallback_key_random_length(
    OlmAccount const * account
);

/** Generates a new fallback key. Only one previous fallback key is
 * stored. Returns olm_error() on error. If the number of random bytes is too
 * small then olm_account_last_error() will be "NOT_ENOUGH_RANDOM". */
OLM_EXPORT size_t olm_account_generate_fallback_key(
    OlmAccount * account,
    void * random, size_t random_length
);

/** The number of bytes needed to hold the fallback key as returned by
 * olm_account_fallback_key. */
OLM_EXPORT size_t olm_account_fallback_key_length(
    OlmAccount const * account
);

/** Deprecated: use olm_account_unpublished_fallback_key instead */
OLM_EXPORT size_t olm_account_fallback_key(
    OlmAccount * account,
    void * fallback_key, size_t fallback_key_size
);

/** The number of bytes needed to hold the unpublished fallback key as returned
 * by olm_account_unpublished fallback_key. */
OLM_EXPORT size_t olm_account_unpublished_fallback_key_length(
    OlmAccount const * account
);

/** Returns the fallback key (if present, and if unpublished) into the
 * fallback_key buffer */
OLM_EXPORT size_t olm_account_unpublished_fallback_key(
    OlmAccount * account,
    void * fallback_key, size_t fallback_key_size
);

/** Forget about the old fallback key.  This should be called once you are
 * reasonably certain that you will not receive any more messages that use
 * the old fallback key (e.g. 5 minutes after the new fallback key has been
 * published).
 */
OLM_EXPORT void olm_account_forget_old_fallback_key(
    OlmAccount * account
);


/** The number of random bytes needed to create an outbound session */
OLM_EXPORT size_t olm_create_outbound_session_random_length(
    OlmSession const * session
);

/** Creates a new out-bound session for sending messages to a given identity_key
 * and one_time_key. Returns olm_error() on failure. If the keys couldn't be
 * decoded as base64 then olm_session_last_error() will be "INVALID_BASE64"
 * If there weren't enough random bytes then olm_session_last_error() will
 * be "NOT_ENOUGH_RANDOM". */
OLM_EXPORT size_t olm_create_outbound_session(
    OlmSession * session,
    OlmAccount const * account,
    void const * their_identity_key, size_t their_identity_key_length,
    void const * their_signing_key, size_t their_signing_key_length,
    void const * their_pre_key, size_t their_pre_key_length,
    void const * their_pre_key_signature, size_t their_pre_key_signature_length,
    void const * their_one_time_key, size_t their_one_time_key_length,
    void * random, size_t random_length
);

/** Creates a new out-bound session for sending messages to a given identity_key.
 * Returns olm_error() on failure. If the keys couldn't be
 * decoded as base64 then olm_session_last_error() will be "INVALID_BASE64"
 * If there weren't enough random bytes then olm_session_last_error() will
 * be "NOT_ENOUGH_RANDOM".
 * It uses the prekey as one time key to perform three steps of DH. */
OLM_EXPORT size_t olm_create_outbound_session_without_otk(
    OlmSession * session,
    OlmAccount const * account,
    void const * their_identity_key, size_t their_identity_key_length,
    void const * their_signing_key, size_t their_signing_key_length,
    void const * their_pre_key, size_t their_pre_key_length,
    void const * their_pre_key_signature, size_t their_pre_key_signature_length,
    void * random, size_t random_length
);

/** Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message. Returns olm_error() on failure. If the base64
 * couldn't be decoded then olm_session_last_error will be "INVALID_BASE64".
 * If the message was for an unsupported protocol version then
 * olm_session_last_error() will be "BAD_MESSAGE_VERSION". If the message
 * couldn't be decoded then olm_session_last_error() will be
 * "BAD_MESSAGE_FORMAT". If the message refers to an unknown one time key,
 * or if the message does not have a one time key and is using an unknown prekey,
 * then olm_session_last_error() will be "BAD_MESSAGE_KEY_ID". */
OLM_EXPORT size_t olm_create_inbound_session(
    OlmSession * session,
    OlmAccount * account,
    void * one_time_key_message, size_t message_length
);

/** Same as olm_create_inbound_session, but ensures that the identity key
 * in the pre-key message matches the expected identity key, supplied via the
 * `their_identity_key` parameter. Fails early if there is no match. */
OLM_EXPORT size_t olm_create_inbound_session_from(
    OlmSession * session,
    OlmAccount * account,
    void const * their_identity_key, size_t their_identity_key_length,
    void * one_time_key_message, size_t message_length
);

/** The length of the buffer needed to return the id for this session. */
OLM_EXPORT size_t olm_session_id_length(
    OlmSession const * session
);

/** An identifier for this session. Will be the same for both ends of the
 * conversation. If the id buffer is too small then olm_session_last_error()
 * will be "OUTPUT_BUFFER_TOO_SMALL". */
OLM_EXPORT size_t olm_session_id(
    OlmSession * session,
    void * id, size_t id_length
);

OLM_EXPORT int olm_session_has_received_message(
    OlmSession const *session
);

OLM_EXPORT int olm_session_is_sender_chain_empty(
    OlmSession const *session
);

/**
 * Write a null-terminated string describing the internal state of an olm
 * session to the buffer provided for debugging and logging purposes. If the
 * buffer is not large enough to hold the entire string, it will be truncated
 * and will end with "...".  A buffer length of 600 will be enough to hold any
 * output.
 */
OLM_EXPORT void olm_session_describe(OlmSession * session, char *buf, size_t buflen);

/** Checks if the PRE_KEY message is for this in-bound session. This can happen
 * if multiple messages are sent to this account before this account sends a
 * message in reply. The one_time_key_message buffer is destroyed. Returns 1 if
 * the session matches. Returns 0 if the session does not match. Returns
 * olm_error() on failure. If the base64 couldn't be decoded then
 * olm_session_last_error will be "INVALID_BASE64".  If the message was for an
 * unsupported protocol version then olm_session_last_error() will be
 * "BAD_MESSAGE_VERSION". If the message couldn't be decoded then then
 * olm_session_last_error() will be "BAD_MESSAGE_FORMAT". */
OLM_EXPORT size_t olm_matches_inbound_session(
    OlmSession * session,
    void * one_time_key_message, size_t message_length
);

/** Checks if the PRE_KEY message is for this in-bound session. This can happen
 * if multiple messages are sent to this account before this account sends a
 * message in reply. The one_time_key_message buffer is destroyed. Returns 1 if
 * the session matches. Returns 0 if the session does not match. Returns
 * olm_error() on failure. If the base64 couldn't be decoded then
 * olm_session_last_error will be "INVALID_BASE64".  If the message was for an
 * unsupported protocol version then olm_session_last_error() will be
 * "BAD_MESSAGE_VERSION". If the message couldn't be decoded then then
 * olm_session_last_error() will be "BAD_MESSAGE_FORMAT". */
OLM_EXPORT size_t olm_matches_inbound_session_from(
    OlmSession * session,
    void const * their_identity_key, size_t their_identity_key_length,
    void * one_time_key_message, size_t message_length
);

/** Removes the one time keys that the session used from the account. Returns
 * olm_error() on failure. If the account doesn't have any matching one time
 * keys then olm_account_last_error() will be "BAD_MESSAGE_KEY_ID".
 * If no one time key was used, then this is a no-op. */
OLM_EXPORT size_t olm_remove_one_time_keys(
    OlmAccount * account,
    OlmSession * session
);

/** The type of the next message that olm_encrypt() will return. Returns
 * OLM_MESSAGE_TYPE_PRE_KEY if the message will be a PRE_KEY message.
 * Returns OLM_MESSAGE_TYPE_MESSAGE if the message will be a normal message.
 * Returns olm_error on failure. */
OLM_EXPORT size_t olm_encrypt_message_type(
    OlmSession const * session
);

/** The number of random bytes needed to encrypt the next message. */
OLM_EXPORT size_t olm_encrypt_random_length(
    OlmSession const * session
);

/** The size of the next message in bytes for the given number of plain-text
 * bytes. */
OLM_EXPORT size_t olm_encrypt_message_length(
    OlmSession const * session,
    size_t plaintext_length
);

/** Encrypts a message using the session. Returns the length of the message in
 * bytes on success. Writes the message as base64 into the message buffer.
 * Returns olm_error() on failure. If the message buffer is too small then
 * olm_session_last_error() will be "OUTPUT_BUFFER_TOO_SMALL". If there
 * weren't enough random bytes then olm_session_last_error() will be
 * "NOT_ENOUGH_RANDOM". */
OLM_EXPORT size_t olm_encrypt(
    OlmSession * session,
    void const * plaintext, size_t plaintext_length,
    void * random, size_t random_length,
    void * message, size_t message_length
);

/** The maximum number of bytes of plain-text a given message could decode to.
 * The actual size could be different due to padding. The input message buffer
 * is destroyed. Returns olm_error() on failure. If the message base64
 * couldn't be decoded then olm_session_last_error() will be
 * "INVALID_BASE64". If the message is for an unsupported version of the
 * protocol then olm_session_last_error() will be "BAD_MESSAGE_VERSION".
 * If the message couldn't be decoded then olm_session_last_error() will be
 * "BAD_MESSAGE_FORMAT". */
OLM_EXPORT size_t olm_decrypt_max_plaintext_length(
    OlmSession * session,
    size_t message_type,
    void * message, size_t message_length
);

/** Decrypts a message using the session. The input message buffer is destroyed.
 * Returns the length of the plain-text on success. Returns olm_error() on
 * failure. If the plain-text buffer is smaller than
 * olm_decrypt_max_plaintext_length() then olm_session_last_error()
 * will be "OUTPUT_BUFFER_TOO_SMALL". If the base64 couldn't be decoded then
 * olm_session_last_error() will be "INVALID_BASE64". If the message is for
 * an unsupported version of the protocol then olm_session_last_error() will
 * be "BAD_MESSAGE_VERSION". If the message couldn't be decoded then
 * olm_session_last_error() will be BAD_MESSAGE_FORMAT".
 * If the MAC on the message was invalid then olm_session_last_error() will
 * be "BAD_MESSAGE_MAC". */
OLM_EXPORT size_t olm_decrypt(
    OlmSession * session,
    size_t message_type,
    void * message, size_t message_length,
    void * plaintext, size_t max_plaintext_length
);

/** The same as decrypt() but allows decrypting only when messages order
 * is maintained, otherwise the last_error will be "MESSAGE_OUT_OF_ORDER". */
OLM_EXPORT size_t olm_decrypt_sequential(
    OlmSession * session,
    size_t message_type,
    void * message, size_t message_length,
    void * plaintext, size_t max_plaintext_length
);

/** The length of the buffer needed to hold the SHA-256 hash. */
OLM_EXPORT size_t olm_sha256_length(
   OlmUtility const * utility
);

/** Calculates the SHA-256 hash of the input and encodes it as base64. If the
 * output buffer is smaller than olm_sha256_length() then
 * olm_utility_last_error() will be "OUTPUT_BUFFER_TOO_SMALL". */
OLM_EXPORT size_t olm_sha256(
    OlmUtility * utility,
    void const * input, size_t input_length,
    void * output, size_t output_length
);

/** Verify an ed25519 signature. If the key was too small then
 * olm_utility_last_error() will be "INVALID_BASE64". If the signature was invalid
 * then olm_utility_last_error() will be "BAD_MESSAGE_MAC". */
OLM_EXPORT size_t olm_ed25519_verify(
    OlmUtility * utility,
    void const * key, size_t key_length,
    void const * message, size_t message_length,
    void * signature, size_t signature_length
);

/** The block below contains only Emscripten-specific functions. */
#ifdef EMSCRIPTEN
/** Function to get the total memory allocated to the Emscripten heap in bytes.  */
OLM_EXPORT unsigned int olm_get_total_memory(void);

/** Function to calculate and return the amount of used memory in bytes. */
OLM_EXPORT int olm_get_used_memory(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* OLM_H_ */
