/*
 * Copyright 2016 OpenMarket Ltd
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

#include "olm_session.h"

using namespace AndroidOlmSdk;

/**
* Init memory allocation for a session creation.<br>
* Make sure releaseSessionJni() is called when one is done with the session instance.
* @return valid memory allocation, NULL otherwise
**/
OlmSession* initializeSessionMemory()
{
    OlmSession* sessionPtr = NULL;
    size_t sessionSize = olm_session_size();

    if(NULL != (sessionPtr=(OlmSession*)malloc(sessionSize)))
    { // init session object
      sessionPtr = olm_session(sessionPtr);
      LOGD("## initializeSessionMemory(): success - OLM session size=%lu",static_cast<long unsigned int>(sessionSize));
    }
    else
    {
      LOGE("## initializeSessionMemory(): failure - OOM");
    }

    return sessionPtr;
}

JNIEXPORT jlong OLM_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## createNewSessionJni(): IN");
    OlmSession* accountPtr = initializeSessionMemory();

    LOGD(" ## createNewSessionJni(): success - accountPtr=%p (jlong)(intptr_t)accountPtr=%lld",accountPtr,(jlong)(intptr_t)accountPtr);
    return (jlong)(intptr_t)accountPtr;
}

JNIEXPORT void OLM_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
  OlmSession* sessionPtr = NULL;

  LOGD("## releaseSessionJni(): IN");

  if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
  {
      LOGE("## releaseSessionJni(): failure - invalid Session ptr=NULL");
  }
  else
  {
    olm_clear_session(sessionPtr);

    // even if free(NULL) does not crash, logs are performed for debug purpose
    free(sessionPtr);
  }
}

/**
* Initialize a new session and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmSession* => jlong) platform independent.
* @return the initialized OlmSession* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong OLM_SESSION_FUNC_DEF(initNewSessionJni)(JNIEnv *env, jobject thiz)
{
    OlmSession* sessionPtr = NULL;

    LOGD("## initNewSessionJni(): OlmSession IN");

    // init account memory allocation
    if(NULL == (sessionPtr = initializeSessionMemory()))
    {
        LOGE(" ## initNewSessionJni(): failure - init session OOM");
    }
    else
    {
       LOGD(" ## initNewSessionJni(): success - OLM session created");
    }

    return (jlong)(intptr_t)sessionPtr;
}

// *********************************************************************
// ********************** OUTBOUND SESSION *****************************
// *********************************************************************
/**
* Create a new in-bound session for sending/receiving messages from an
* incoming PRE_KEY message.<br> The recipient is defined as the entity
* with whom the session is established.
* @param aOlmAccountId account instance
* @param aTheirIdentityKey the identity key of the recipient
* @param aTheirOneTimeKey the one time key of the recipient
* @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
**/
JNIEXPORT jint OLM_SESSION_FUNC_DEF(initOutboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aTheirOneTimeKey)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession* sessionPtr = NULL;
    OlmAccount* accountPtr = NULL;
    const char* theirIdentityKeyPtr = NULL;
    const char* theirOneTimeKeyPtr = NULL;
    uint8_t *randomBuffPtr = NULL;
    size_t sessionResult;

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## initOutboundSessionJni(): failure - invalid Session ptr=NULL");
    }
    else if(NULL == (accountPtr = (OlmAccount*)aOlmAccountId))
    {
        LOGE("## initOutboundSessionJni(): failure - invalid Account ptr=NULL");
    }
    else if((0==aTheirIdentityKey) || (0==aTheirOneTimeKey))
    {
        LOGE("## initOutboundSessionJni(): failure - invalid keys");
    }
    else
    {   // allocate random buffer
        size_t randomSize = olm_create_outbound_session_random_length(sessionPtr);
        LOGD("## initOutboundSessionJni(): randomSize=%lu",static_cast<long unsigned int>(randomSize));
        if((0!=randomSize) && !setRandomInBuffer(env, &randomBuffPtr, randomSize))
        {
            LOGE("## initOutboundSessionJni(): failure - random buffer init");
        }
        else
        {   // convert identity & one time keys to C strings
            if(NULL == (theirIdentityKeyPtr = env->GetStringUTFChars(aTheirIdentityKey, 0)))
            {
                LOGE("## initOutboundSessionJni(): failure - identityKey JNI allocation OOM");
            }
            else if(NULL == (theirOneTimeKeyPtr = env->GetStringUTFChars(aTheirOneTimeKey, 0)))
            {
                LOGE("## initOutboundSessionJni(): failure - one time Key JNI allocation OOM");
            }
            else
            {
                size_t theirIdentityKeyLength = (size_t)env->GetStringUTFLength(aTheirIdentityKey);
                size_t theirOneTimeKeyLength  = (size_t)env->GetStringUTFLength(aTheirOneTimeKey);
                LOGD("## initOutboundSessionJni(): identityKey=%s oneTimeKey=%s",theirIdentityKeyPtr,theirOneTimeKeyPtr);

                sessionResult = olm_create_outbound_session(sessionPtr,
                                                            accountPtr,
                                                            theirIdentityKeyPtr,
                                                            theirIdentityKeyLength,
                                                            theirOneTimeKeyPtr,
                                                            theirOneTimeKeyLength,
                                                            (void*)randomBuffPtr,
                                                            randomSize);
                if(sessionResult == olm_error()) {
                    LOGE("## initOutboundSessionJni(): failure - session creation  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
                }
                else
                {
                    retCode = ERROR_CODE_OK;
                    LOGD("## initOutboundSessionJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
                }
            }
        }
    }

     // **** free mem alloc ***
     if(NULL!= randomBuffPtr)
     {
         free(randomBuffPtr);
     }

     if(NULL!= theirIdentityKeyPtr)
     {
         env->ReleaseStringUTFChars(aTheirIdentityKey, theirIdentityKeyPtr);
     }

     if(NULL!= theirOneTimeKeyPtr)
     {
         env->ReleaseStringUTFChars(aTheirOneTimeKey, theirOneTimeKeyPtr);
     }

    return retCode;
}


// *********************************************************************
// *********************** INBOUND SESSION *****************************
// *********************************************************************
/**
 * Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message.<br>
 * @param aOlmAccountId account instance
 * @param aOneTimeKeyMsg PRE_KEY message
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint OLM_SESSION_FUNC_DEF(initInboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aOneTimeKeyMsg)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession *sessionPtr = NULL;
    OlmAccount *accountPtr = NULL;
    const char *messagePtr = NULL;
    size_t sessionResult;

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## initInboundSessionJni(): failure - invalid Session ptr=NULL");
    }
    else if(NULL == (accountPtr = (OlmAccount*)aOlmAccountId))
    {
        LOGE("## initInboundSessionJni(): failure - invalid Account ptr=NULL");
    }
    else if(0==aOneTimeKeyMsg)
    {
        LOGE("## initInboundSessionJni(): failure - invalid message");
    }
    else
    {   // convert message to C strings
        if(NULL == (messagePtr = env->GetStringUTFChars(aOneTimeKeyMsg, 0)))
        {
            LOGE("## initInboundSessionJni(): failure - message JNI allocation OOM");
        }
        else
        {
            size_t messageLength = (size_t)env->GetStringUTFLength(aOneTimeKeyMsg);
            LOGD("## initInboundSessionJni(): messageLength=%lu message=%s", static_cast<long unsigned int>(messageLength), messagePtr);

            sessionResult = olm_create_inbound_session(sessionPtr, accountPtr, (void*)messagePtr , messageLength);
            if(sessionResult == olm_error()) {
                LOGE("## initInboundSessionJni(): failure - init inbound session creation  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
            }
            else
            {
                retCode = ERROR_CODE_OK;
                LOGD("## initInboundSessionJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
            }

            // free local alloc
            env->ReleaseStringUTFChars(aOneTimeKeyMsg, messagePtr);
        }
    }
    return retCode;
}

/**
 * Create a new in-bound session for sending/receiving messages from an
 * incoming PRE_KEY message based on the recipient identity key.<br>
 * @param aOlmAccountId account instance
 * @param aTheirIdentityKey the identity key of the recipient
 * @param aOneTimeKeyMsg encrypted message
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint OLM_SESSION_FUNC_DEF(initInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession *sessionPtr = NULL;
    OlmAccount *accountPtr = NULL;
    const char *messagePtr = NULL;
    const char *theirIdentityKeyPtr = NULL;
    size_t sessionResult;

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - invalid Session ptr=NULL");
    }
    else if(NULL == (accountPtr = (OlmAccount*)aOlmAccountId))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - invalid Account ptr=NULL");
    }
    else if(0 == aTheirIdentityKey)
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - invalid theirIdentityKey");
    }
    else if(0==aOneTimeKeyMsg)
    {
        LOGE("## initInboundSessionJni(): failure - invalid one time key message");
    }
    else if(NULL == (messagePtr = env->GetStringUTFChars(aOneTimeKeyMsg, 0)))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - message JNI allocation OOM");
    }
    else if(NULL == (theirIdentityKeyPtr = env->GetStringUTFChars(aTheirIdentityKey, 0)))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - theirIdentityKey JNI allocation OOM");
    }
    else
    {
        size_t messageLength = (size_t)env->GetStringUTFLength(aOneTimeKeyMsg);
        size_t theirIdentityKeyLength = (size_t)env->GetStringUTFLength(aTheirIdentityKey);

        LOGD("## initInboundSessionFromIdKeyJni(): message=%s messageLength=%lu",messagePtr,static_cast<long unsigned int>(messageLength));

        sessionResult = olm_create_inbound_session_from(sessionPtr, accountPtr, theirIdentityKeyPtr, theirIdentityKeyLength, (void*)messagePtr , messageLength);
        if(sessionResult == olm_error()) {
            LOGE("## initInboundSessionFromIdKeyJni(): failure - init inbound session creation  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## initInboundSessionFromIdKeyJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
        }
     }

     // free local alloc
     if(NULL!= messagePtr)
     {
         env->ReleaseStringUTFChars(aOneTimeKeyMsg, messagePtr);
     }
     if(NULL!= theirIdentityKeyPtr)
     {
         env->ReleaseStringUTFChars(aTheirIdentityKey, theirIdentityKeyPtr);
     }

    return retCode;
}

/**
 * Checks if the PRE_KEY message is for this in-bound session.<br>
 * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
 * @param aOneTimeKeyMsg PRE KEY message
 * @return ERROR_CODE_OK if match, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint OLM_SESSION_FUNC_DEF(matchesInboundSessionJni)(JNIEnv *env, jobject thiz, jstring aOneTimeKeyMsg)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession *sessionPtr = NULL;
    const char *messagePtr = NULL;

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## matchesInboundSessionJni(): failure - invalid Session ptr=NULL");
    }
    else if(0==aOneTimeKeyMsg)
    {
        LOGE("## matchesInboundSessionJni(): failure - invalid one time key message");
    }
    else if(NULL == (messagePtr = env->GetStringUTFChars(aOneTimeKeyMsg, 0)))
    {
        LOGE("## matchesInboundSessionJni(): failure - one time key JNI allocation OOM");
    }
    else
    {
        size_t messageLength = (size_t)env->GetStringUTFLength(aOneTimeKeyMsg);

        size_t matchResult = olm_matches_inbound_session(sessionPtr, (void*)messagePtr , messageLength);
        //if(matchResult == olm_error()) {
        // for now olm_matches_inbound_session() returns 1 when it succeeds, otherwise 1- or 0
        if(matchResult != 1) {
            LOGE("## matchesInboundSessionJni(): failure - no match  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## matchesInboundSessionJni(): success - result=%lu", static_cast<long unsigned int>(matchResult));
        }
    }

    // free local alloc
    if(NULL!= messagePtr)
    {
     env->ReleaseStringUTFChars(aOneTimeKeyMsg, messagePtr);
    }

    return retCode;
}


/**
 * Checks if the PRE_KEY message is for this in-bound session based on the sender identity key.<br>
 * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
 * @param aTheirIdentityKey the identity key of the sender
 * @param aOneTimeKeyMsg PRE KEY message
 * @return ERROR_CODE_OK if match, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint JNICALL OLM_SESSION_FUNC_DEF(matchesInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession *sessionPtr = NULL;
    const char *messagePtr = NULL;
    const char *theirIdentityKeyPtr = NULL;

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - invalid Session ptr=NULL");
    }
    else if(0 == aTheirIdentityKey)
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - invalid theirIdentityKey");
    }
    else if(NULL == (theirIdentityKeyPtr = env->GetStringUTFChars(aTheirIdentityKey, 0)))
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - theirIdentityKey JNI allocation OOM");
    }
    else if(0==aOneTimeKeyMsg)
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - invalid one time key message");
    }
    else if(NULL == (messagePtr = env->GetStringUTFChars(aOneTimeKeyMsg, 0)))
    {
        LOGE("## matchesInboundSessionFromIdKeyJni(): failure - one time key JNI allocation OOM");
    }
    else
    {
        size_t identityKeyLength = (size_t)env->GetStringUTFLength(aTheirIdentityKey);
        size_t messageLength = (size_t)env->GetStringUTFLength(aOneTimeKeyMsg);

        size_t matchResult = olm_matches_inbound_session_from(sessionPtr, (void const *)theirIdentityKeyPtr, identityKeyLength, (void*)messagePtr , messageLength);
        //if(matchResult == olm_error()) {
        // for now olm_matches_inbound_session() returns 1 when it succeeds, otherwise 1- or 0
        if(matchResult != 1) {
            LOGE("## matchesInboundSessionFromIdKeyJni(): failure - no match  Msg=%s",(const char *)olm_session_last_error(sessionPtr));
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## matchesInboundSessionFromIdKeyJni(): success - result=%lu", static_cast<long unsigned int>(matchResult));
        }
    }

    // free local alloc
    if(NULL!= theirIdentityKeyPtr)
    {
     env->ReleaseStringUTFChars(aTheirIdentityKey, theirIdentityKeyPtr);
    }

    if(NULL!= messagePtr)
    {
     env->ReleaseStringUTFChars(aOneTimeKeyMsg, messagePtr);
    }

    return retCode;
}


/**
 * Encrypt a message using the session.<br>
 * @param aClearMsg clear text message
 * @param [out] aEncryptedMsg ciphered message
 * @return ERROR_CODE_OK if encrypt operation succeed, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint OLM_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jstring aClearMsg, jobject aEncryptedMsg)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession *sessionPtr = NULL;
    const char *clearMsgPtr = NULL;
    uint8_t *randomBuffPtr = NULL;
    void *encryptedMsgPtr = NULL;
    jclass encryptedMsgJClass = 0;
    jfieldID encryptedMsgFieldId;
    jfieldID typeMsgFieldId;

    LOGD("## encryptMessageJni(): IN ");

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## encryptMessageJni(): failure - invalid Session ptr=NULL");
    }
    else if(0 == aClearMsg)
    {
        LOGE("## encryptMessageJni(): failure - invalid clear message");
    }
    else if(0 == aEncryptedMsg)
    {
        LOGE("## encryptMessageJni(): failure - invalid encrypted message");
    }
    else if(NULL == (clearMsgPtr = env->GetStringUTFChars(aClearMsg, 0)))
    {
        LOGE("## encryptMessageJni(): failure - clear message JNI allocation OOM");
    }
    else if(0 == (encryptedMsgJClass = env->GetObjectClass(aEncryptedMsg)))
    {
        LOGE("## encryptMessageJni(): failure - unable to get crypted message class");
    }
    else if(0 == (encryptedMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mCipherText","Ljava/lang/String;")))
    {
        LOGE("## encryptMessageJni(): failure - unable to get message field");
    }
    else if(0 == (typeMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mType","J")))
    {
        LOGE("## encryptMessageJni(): failure - unable to get message type field");
    }
    else
    {
        // get message type
        size_t messageType = olm_encrypt_message_type(sessionPtr);

        // compute random buffer
        // Note: olm_encrypt_random_length() can return 0, which means
        // it just does not need new random data to encrypt a new message
        size_t randomLength = olm_encrypt_random_length(sessionPtr);
        LOGD("## encryptMessageJni(): randomLength=%lu", static_cast<long unsigned int>(randomLength));
        if((0!=randomLength) && !setRandomInBuffer(env, &randomBuffPtr, randomLength))
        {
            LOGE("## encryptMessageJni(): failure - random buffer init");
        }
        else
        {
            // alloc buffer for encrypted message
            size_t clearMsgLength = (size_t)env->GetStringUTFLength(aClearMsg);
            size_t encryptedMsgLength = olm_encrypt_message_length(sessionPtr, clearMsgLength);
            if(NULL == (encryptedMsgPtr = (void*)malloc((encryptedMsgLength+1)*sizeof(uint8_t))))
            {
                LOGE("## encryptMessageJni(): failure - encryptedMsgPtr buffer OOM");
            }
            else
            {
                if(0==randomLength)
                {
                    LOGW("## encryptMessageJni(): random buffer is not required");
                }

                LOGD("## encryptMessageJni(): messageType=%lu randomLength=%lu clearMsgLength=%lu encryptedMsgLength=%lu",static_cast<long unsigned int>(messageType),static_cast<long unsigned int>(randomLength), static_cast<long unsigned int>(clearMsgLength), static_cast<long unsigned int>(encryptedMsgLength));
                // encrypt message
                size_t result = olm_encrypt(sessionPtr,
                                            (void const *)clearMsgPtr,
                                            clearMsgLength,
                                            randomBuffPtr,
                                            randomLength,
                                            encryptedMsgPtr,
                                            encryptedMsgLength);
                if(result == olm_error())
                {
                    LOGE("## encryptMessageJni(): failure - Msg=%s",(const char *)olm_session_last_error(sessionPtr));
                }
                else
                {
                    // update encrypted buffer size
                    (static_cast<char*>(encryptedMsgPtr))[result] = static_cast<char>('\0');

                    // update message type: PRE KEY or normal
                    env->SetLongField(aEncryptedMsg, typeMsgFieldId, (jlong)messageType);

                    // update message: encryptedMsgPtr => encryptedJstring
                    jstring encryptedJstring = env->NewStringUTF((const char*)encryptedMsgPtr);
                    env->SetObjectField(aEncryptedMsg, encryptedMsgFieldId, (jobject)encryptedJstring);

                    retCode = ERROR_CODE_OK;
                    LOGD("## encryptMessageJni(): success - result=%lu Type=%lu utfLength=%lu encryptedMsg=%s", static_cast<long unsigned int>(result), static_cast<long unsigned int>(messageType), static_cast<long unsigned int>((size_t)env->GetStringUTFLength(encryptedJstring)), (const char*)encryptedMsgPtr);
                }
            }
        }
    }

    // free alloc
    if(NULL != clearMsgPtr)
    {
     env->ReleaseStringUTFChars(aClearMsg, clearMsgPtr);
    }

    if(NULL != randomBuffPtr)
    {
        free(randomBuffPtr);
    }

    if(NULL != encryptedMsgPtr)
    {
        free(encryptedMsgPtr);
    }

    return retCode;
}


/**
 * Decrypt a message using the session.<br>
 * @param aEncryptedMsg message to decrypt
 * @return decrypted message if operation succeed, null otherwise
 */
JNIEXPORT jstring OLM_SESSION_FUNC_DEF(decryptMessageJni)(JNIEnv *env, jobject thiz, jobject aEncryptedMsg, jboolean aIsUtf8ConversionRequired)
{
    jstring decryptedMsgRetValue = 0;
    jclass encryptedMsgJClass = 0;
    jstring encryptedMsgJstring = 0; // <= obtained from encryptedMsgFieldId
    // field IDs
    jfieldID encryptedMsgFieldId;
    jfieldID typeMsgFieldId;
    // ptrs
    OlmSession *sessionPtr = NULL;
    const char *encryptedMsgPtr = NULL; // <= obtained from encryptedMsgJstring
    uint8_t *plainTextMsgPtr = NULL;
    char *tempEncryptedPtr = NULL;

    LOGD("## decryptMessageJni(): IN - OlmSession");

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## decryptMessageJni(): failure - invalid Session ptr=NULL");
    }
    else if(0 == aEncryptedMsg)
    {
        LOGE("## decryptMessageJni(): failure - invalid encrypted message");
    }
    else if(0 == (encryptedMsgJClass = env->GetObjectClass(aEncryptedMsg)))
    {
        LOGE("## decryptMessageJni(): failure - unable to get encrypted message class");
    }
    else if(0 == (encryptedMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mCipherText","Ljava/lang/String;")))
    {
        LOGE("## decryptMessageJni(): failure - unable to get message field");
    }
    else if(0 == (typeMsgFieldId = env->GetFieldID(encryptedMsgJClass,"mType","J")))
    {
        LOGE("## decryptMessageJni(): failure - unable to get message type field");
    }
    else if(0 == (encryptedMsgJstring = (jstring)env->GetObjectField(aEncryptedMsg, encryptedMsgFieldId)))
    {
        LOGE("## decryptMessageJni(): failure - JNI encrypted object ");
    }
    else if(0 == (encryptedMsgPtr = env->GetStringUTFChars(encryptedMsgJstring, 0)))
    {
        LOGE("## decryptMessageJni(): failure - encrypted message JNI allocation OOM");
    }
    else
    {
        // get message type
        size_t encryptedMsgType = (size_t)env->GetLongField(aEncryptedMsg, typeMsgFieldId);
        // get encrypted message length
        size_t encryptedMsgLength = (size_t)env->GetStringUTFLength(encryptedMsgJstring);

        // create a dedicated temp buffer to be used in next Olm API calls
        tempEncryptedPtr = static_cast<char*>(malloc(encryptedMsgLength*sizeof(uint8_t)));
        memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
        LOGD("## decryptMessageJni(): MsgType=%lu encryptedMsgLength=%lu encryptedMsg=%s",static_cast<long unsigned int>(encryptedMsgType),static_cast<long unsigned int>(encryptedMsgLength),encryptedMsgPtr);

        // get max plaintext length
        size_t maxPlainTextLength = olm_decrypt_max_plaintext_length(sessionPtr,
                                                                     static_cast<size_t>(encryptedMsgType),
                                                                     static_cast<void*>(tempEncryptedPtr),
                                                                     encryptedMsgLength);
        // Note: tempEncryptedPtr is destroyed by olm_decrypt_max_plaintext_length()

        if(maxPlainTextLength == olm_error())
        {
            LOGE("## decryptMessageJni(): failure - olm_decrypt_max_plaintext_length Msg=%s",(const char *)olm_session_last_error(sessionPtr));
        }
        else
        {
            LOGD("## decryptMessageJni(): maxPlaintextLength=%lu",static_cast<long unsigned int>(maxPlainTextLength));

            // allocate output decrypted message
            plainTextMsgPtr = static_cast<uint8_t*>(malloc((maxPlainTextLength+1)*sizeof(uint8_t)));

            // decrypt, but before reload encrypted buffer (previous one was destroyed)
            memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
            size_t plaintextLength = olm_decrypt(sessionPtr,
                                                 encryptedMsgType,
                                                 (void*)tempEncryptedPtr,
                                                 encryptedMsgLength,
                                                 plainTextMsgPtr,
                                                 maxPlainTextLength);
            if(plaintextLength == olm_error())
            {
                LOGE("## decryptMessageJni(): failure - olm_decrypt Msg=%s",(const char *)olm_session_last_error(sessionPtr));
            }
            else
            {
                // UTF-8 conversion workaround for issue on Android versions older than Marshmallow (23)
                if(aIsUtf8ConversionRequired)
                {
                    decryptedMsgRetValue = javaCStringToUtf8(env, plainTextMsgPtr, plaintextLength);
                    if(0 == decryptedMsgRetValue)
                    {
                        LOGE(" ## decryptMessageJni(): UTF-8 Conversion failure - javaCStringToUtf8() returns null");
                    }
                    else
                    {
                        LOGD(" ## decryptMessageJni(): UTF-8 Conversion - decrypted returnedLg=%lu OK",static_cast<long unsigned int>(plaintextLength));
                    }
                }
                else
                {
                    // update decrypted buffer size
                    plainTextMsgPtr[plaintextLength] = static_cast<char>('\0');

                    LOGD("## decryptMessageJni(): decrypted returnedLg=%lu plainTextMsgPtr=%s",static_cast<long unsigned int>(plaintextLength), (char*)(plainTextMsgPtr));
                    decryptedMsgRetValue = env->NewStringUTF((const char*)(plainTextMsgPtr));
                }
            }
        }
    }

    // free alloc
    if(NULL != encryptedMsgPtr)
    {
        env->ReleaseStringUTFChars(encryptedMsgJstring, encryptedMsgPtr);
    }

    if(NULL != tempEncryptedPtr)
    {
        free(tempEncryptedPtr);
    }

    if(NULL != plainTextMsgPtr)
    {
        free(plainTextMsgPtr);
    }

    return decryptedMsgRetValue;
}


/**
* Get the session identifier for this session.
* @return the session identifier if operation succeed, null otherwise
*/
JNIEXPORT jstring OLM_SESSION_FUNC_DEF(getSessionIdentifierJni)(JNIEnv *env, jobject thiz)
{
    OlmSession *sessionPtr = NULL;
    void *sessionIdPtr = NULL;
    jstring returnValueStr=0;

    LOGD("## getSessionIdentifierJni(): IN ");

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## getSessionIdentifierJni(): failure - invalid Session ptr=NULL");
    }
    else
    {
        // get the size to alloc to contain the id
        size_t lengthSessionId = olm_session_id_length(sessionPtr);
        LOGD("## getSessionIdentifierJni(): lengthSessionId=%lu",static_cast<long unsigned int>(lengthSessionId));

        if(NULL == (sessionIdPtr = (void*)malloc((lengthSessionId+1)*sizeof(uint8_t))))
        {
           LOGE("## getSessionIdentifierJni(): failure - identifier allocation OOM");
        }
        else
        {
            size_t result = olm_session_id(sessionPtr, sessionIdPtr, lengthSessionId);

            if (result == olm_error())
            {
                LOGE("## getSessionIdentifierJni(): failure - get session identifier failure Msg=%s",(const char *)olm_session_last_error(sessionPtr));
            }
            else
            {
                // update length
                (static_cast<char*>(sessionIdPtr))[result] = static_cast<char>('\0');

                LOGD("## getSessionIdentifierJni(): success - result=%lu sessionId=%s",static_cast<long unsigned int>(result), (char*)sessionIdPtr);
                returnValueStr = env->NewStringUTF((const char*)sessionIdPtr);
            }
            free(sessionIdPtr);
        }
    }

    return returnValueStr;
}


/**
* Serialize and encrypt session instance into a base64 string.<br>
* @param aKey key used to encrypt the serialized session data
* @param[out] aErrorMsg error message set if operation failed
* @return a base64 string if operation succeed, null otherwise
**/
JNIEXPORT jstring OLM_SESSION_FUNC_DEF(serializeDataWithKeyJni)(JNIEnv *env, jobject thiz, jstring aKey, jobject aErrorMsg)
{
    jstring pickledDataRetValue = 0;
    jclass errorMsgJClass = 0;
    jmethodID errorMsgMethodId = 0;
    jstring errorJstring = 0;
    const char *keyPtr = NULL;
    void *pickledPtr = NULL;
    OlmSession* sessionPtr = NULL;

    LOGD("## serializeDataWithKeyJni(): IN");

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid session ptr");
    }
    else if(0 == aKey)
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid key");
    }
    else if(0 == aErrorMsg)
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid error object");
    }
    else if(0 == (errorMsgJClass = env->GetObjectClass(aErrorMsg)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - unable to get error class");
    }
    else if(0 == (errorMsgMethodId = env->GetMethodID(errorMsgJClass, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;")))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - unable to get error method ID");
    }
    else if(NULL == (keyPtr = env->GetStringUTFChars(aKey, 0)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - keyPtr JNI allocation OOM");
    }
    else
    {
        size_t pickledLength = olm_pickle_session_length(sessionPtr);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        LOGD(" ## serializeDataWithKeyJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## serializeDataWithKeyJni(): key=%s",(char const *)keyPtr);

        if(NULL == (pickledPtr = (void*)malloc((pickledLength+1)*sizeof(uint8_t))))
        {
            LOGE(" ## serializeDataWithKeyJni(): failure - pickledPtr buffer OOM");
        }
        else
        {
            size_t result = olm_pickle_session(sessionPtr,
                                              (void const *)keyPtr,
                                              keyLength,
                                              (void*)pickledPtr,
                                              pickledLength);
            if(result == olm_error())
            {
                const char *errorMsgPtr = olm_session_last_error(sessionPtr);
                LOGE(" ## serializeDataWithKeyJni(): failure - olm_pickle_session() Msg=%s",errorMsgPtr);

                if(0 != (errorJstring = env->NewStringUTF(errorMsgPtr)))
                {
                    env->CallObjectMethod(aErrorMsg, errorMsgMethodId, errorJstring);
                }
            }
            else
            {
                // build success output
                (static_cast<char*>(pickledPtr))[pickledLength] = static_cast<char>('\0');
                pickledDataRetValue = env->NewStringUTF((const char*)pickledPtr);
                LOGD(" ## serializeDataWithKeyJni(): success - result=%lu pickled=%s", static_cast<long unsigned int>(result), static_cast<char*>(pickledPtr));
            }
        }
    }

    // free alloc
    if(NULL != keyPtr)
    {
     env->ReleaseStringUTFChars(aKey, keyPtr);
    }

    if(NULL != pickledPtr)
    {
        free(pickledPtr);
    }

    return pickledDataRetValue;
}


JNIEXPORT jstring OLM_SESSION_FUNC_DEF(initWithSerializedDataJni)(JNIEnv *env, jobject thiz, jstring aSerializedData, jstring aKey)
{
    OlmSession* sessionPtr = NULL;
    jstring errorMessageRetValue = 0;
    const char *keyPtr = NULL;
    const char *pickledPtr = NULL;

    LOGD("## initWithSerializedDataJni(): IN");

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE(" ## initWithSerializedDataJni(): failure - session failure OOM");
    }
    else if(0 == aKey)
    {
        LOGE(" ## initWithSerializedDataJni(): failure - invalid key");
    }
    else if(0 == aSerializedData)
    {
        LOGE(" ## initWithSerializedDataJni(): failure - serialized data");
    }
    else if(NULL == (keyPtr = env->GetStringUTFChars(aKey, 0)))
    {
        LOGE(" ## initWithSerializedDataJni(): failure - keyPtr JNI allocation OOM");
    }
    else if(NULL == (pickledPtr = env->GetStringUTFChars(aSerializedData, 0)))
    {
        LOGE(" ## initWithSerializedDataJni(): failure - pickledPtr JNI allocation OOM");
    }
    else
    {
        size_t pickledLength = (size_t)env->GetStringUTFLength(aSerializedData);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        LOGD(" ## initWithSerializedDataJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## initWithSerializedDataJni(): key=%s",(char const *)keyPtr);
        LOGD(" ## initWithSerializedDataJni(): pickled=%s",(char const *)pickledPtr);

        size_t result = olm_unpickle_session(sessionPtr,
                                             (void const *)keyPtr,
                                             keyLength,
                                             (void*)pickledPtr,
                                             pickledLength);
        if(result == olm_error())
        {
            const char *errorMsgPtr = olm_session_last_error(sessionPtr);
            LOGE(" ## initWithSerializedDataJni(): failure - olm_unpickle_account() Msg=%s",errorMsgPtr);
            errorMessageRetValue = env->NewStringUTF(errorMsgPtr);
        }
        else
        {
            LOGD(" ## initWithSerializedDataJni(): success - result=%lu ", static_cast<long unsigned int>(result));
        }

    }

    // free alloc
    if(NULL != keyPtr)
    {
        env->ReleaseStringUTFChars(aKey, keyPtr);
    }

    if(NULL != pickledPtr)
    {
        env->ReleaseStringUTFChars(aSerializedData, pickledPtr);
    }

    return errorMessageRetValue;
}