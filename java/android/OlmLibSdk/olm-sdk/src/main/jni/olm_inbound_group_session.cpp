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

#include "olm_inbound_group_session.h"

using namespace AndroidOlmSdk;

/**
 * Release the session allocation made by initializeInboundGroupSessionMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_INBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
  OlmInboundGroupSession* sessionPtr = NULL;

  LOGD("## releaseSessionJni(): intbound group session");

  if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
  {
      LOGE("## releaseSessionJni(): failure - invalid inbound group session instance");
  }
  else
  {
    LOGD(" ## releaseSessionJni(): sessionPtr=%p",sessionPtr);

    size_t retCode = olm_clear_inbound_group_session(sessionPtr);
    LOGI(" ## releaseSessionJni(): clear_inbound_group_session=%lu",retCode);

    LOGD(" ## releaseSessionJni(): free IN");
    free(sessionPtr);
    LOGD(" ## releaseSessionJni(): free OUT");
  }
}

/**
* Initialize a new inbound group session and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmInboundGroupSession* => jlong) platform independent.
* @return the initialized OlmInboundGroupSession* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong OLM_INBOUND_GROUP_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz)
{
    OlmInboundGroupSession* sessionPtr = NULL;
    size_t sessionSize = 0;

    LOGD("## createNewSessionJni(): inbound group session IN");
    sessionSize = olm_inbound_group_session_size();

    if(0 == sessionSize)
    {
        LOGE(" ## createNewSessionJni(): failure - inbound group session size = 0");
    }
    else if(NULL != (sessionPtr=(OlmInboundGroupSession*)malloc(sessionSize)))
    {
      sessionPtr = olm_inbound_group_session(sessionPtr);
      LOGD(" ## createNewSessionJni(): success - inbound group session size=%lu",sessionSize);
    }
    else
    {
      LOGE(" ## createNewSessionJni(): failure - inbound group session OOM");
    }

    return (jlong)(intptr_t)sessionPtr;
}

/**
 * Create a new in-bound session.<br>
 * @param aSessionKey session key from an outbound session
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint OLM_INBOUND_GROUP_SESSION_FUNC_DEF(initInboundGroupSessionWithSessionKeyJni)(JNIEnv *env, jobject thiz, jstring aSessionKey)
{
    jint retCode = ERROR_CODE_KO;
    OlmInboundGroupSession *sessionPtr = NULL;
    const uint8_t *sessionKeyPtr = NULL;
    size_t sessionResult;

    LOGD("## initInboundGroupSessionWithSessionKeyJni(): inbound group session IN");

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## initInboundGroupSessionWithSessionKeyJni(): failure - invalid inbound group session instance");
    }
    else if(0 == aSessionKey)
    {
        LOGE(" ## initInboundGroupSessionWithSessionKeyJni(): failure - invalid aSessionKey");
    }
    else if(NULL == (sessionKeyPtr = (const uint8_t *)env->GetStringUTFChars(aSessionKey, 0)))
    {
        LOGE(" ## initInboundSessionFromIdKeyJni(): failure - session key JNI allocation OOM");
    }
    else
    {
        size_t sessionKeyLength = (size_t)env->GetStringUTFLength(aSessionKey);
        LOGD(" ## initInboundSessionFromIdKeyJni(): sessionKeyLength=%lu",sessionKeyLength);

        sessionResult = olm_init_inbound_group_session(sessionPtr, sessionKeyPtr, sessionKeyLength);
        if(sessionResult == olm_error()) {
            const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
            LOGE(" ## initInboundSessionFromIdKeyJni(): failure - init inbound session creation  Msg=%s",errorMsgPtr);
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD(" ## initInboundSessionFromIdKeyJni(): success - result=%lu", sessionResult);
        }
     }

     // free local alloc
     if(NULL!= sessionKeyPtr)
     {
         env->ReleaseStringUTFChars(aSessionKey, (const char*)sessionKeyPtr);
     }

    return retCode;
}


/**
* Get a base64-encoded identifier for this inbound group session.
*/
JNIEXPORT jstring OLM_INBOUND_GROUP_SESSION_FUNC_DEF(sessionIdentifierJni)(JNIEnv *env, jobject thiz)
{
    OlmInboundGroupSession *sessionPtr = NULL;
    uint8_t *sessionIdPtr = NULL;
    jstring returnValueStr=0;

    LOGD("## sessionIdentifierJni(): inbound group session IN");

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## sessionIdentifierJni(): failure - invalid inbound group session instance");
    }
    else
    {
        // get the size to alloc
        size_t lengthSessionId = olm_inbound_group_session_id_length(sessionPtr);
        LOGD(" ## sessionIdentifierJni(): inbound group session lengthSessionId=%lu",lengthSessionId);

        if(NULL == (sessionIdPtr = (uint8_t*)malloc((lengthSessionId+1)*sizeof(uint8_t))))
        {
           LOGE(" ## sessionIdentifierJni(): failure - inbound group session identifier allocation OOM");
        }
        else
        {
            size_t result = olm_inbound_group_session_id(sessionPtr, sessionIdPtr, lengthSessionId);
            if (result == olm_error())
            {
                LOGE(" ## sessionIdentifierJni(): failure - get inbound group session identifier failure Msg=%s",(const char *)olm_inbound_group_session_last_error(sessionPtr));
            }
            else
            {
                // update length
                sessionIdPtr[result] = static_cast<char>('\0');
                LOGD(" ## sessionIdentifierJni(): success - inbound group session result=%lu sessionId=%s",result, (char*)sessionIdPtr);
                returnValueStr = env->NewStringUTF((const char*)sessionIdPtr);
            }
            free(sessionIdPtr);
        }
    }

    return returnValueStr;
}


JNIEXPORT jstring OLM_INBOUND_GROUP_SESSION_FUNC_DEF(decryptMessageJni)(JNIEnv *env, jobject thiz, jstring aEncryptedMsg)
{
    jstring decryptedMsgRetValue = 0;
    OlmInboundGroupSession *sessionPtr = NULL;
    const char *encryptedMsgPtr = NULL;
    uint8_t *plainTextMsgPtr = NULL;
    uint8_t *tempEncryptedPtr = NULL;

    LOGD("## decryptMessageJni(): inbound group session IN");

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## decryptMessageJni(): failure - invalid inbound group session ptr=NULL");
    }
    else if(0 == aEncryptedMsg)
    {
        LOGE(" ## decryptMessageJni(): failure - invalid encrypted message");
    }
    else if(0 == (encryptedMsgPtr = env->GetStringUTFChars(aEncryptedMsg, 0)))
    {
        LOGE(" ## decryptMessageJni(): failure - encrypted message JNI allocation OOM");
    }
    else
    {
        // get encrypted message length
        size_t encryptedMsgLength = (size_t)env->GetStringUTFLength(aEncryptedMsg);

        // create a dedicated temp buffer to be used in next Olm API calls
        if(NULL == (tempEncryptedPtr = static_cast<uint8_t*>(malloc(encryptedMsgLength*sizeof(uint8_t)))))
        {
            LOGE(" ## decryptMessageJni(): failure - tempEncryptedPtr allocation OOM");
        }
        else
        {
            memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
            LOGD(" ## decryptMessageJni(): encryptedMsgLength=%lu encryptedMsg=%s",encryptedMsgLength,encryptedMsgPtr);

            // get max plaintext length
            size_t maxPlainTextLength = olm_group_decrypt_max_plaintext_length(sessionPtr,
                                                                               tempEncryptedPtr,
                                                                               encryptedMsgLength);
            if(maxPlainTextLength == olm_error())
            {
                LOGE(" ## decryptMessageJni(): failure - olm_group_decrypt_max_plaintext_length Msg=%s",(const char *)olm_inbound_group_session_last_error(sessionPtr));
            }
            else
            {
                LOGD(" ## decryptMessageJni(): maxPlaintextLength=%lu",maxPlainTextLength);

                // allocate output decrypted message
                plainTextMsgPtr = static_cast<uint8_t*>(malloc((maxPlainTextLength+1)*sizeof(uint8_t)));

                // decrypt, but before reload encrypted buffer (previous one was destroyed)
                memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
                size_t plaintextLength = olm_group_decrypt(sessionPtr,
                                                           tempEncryptedPtr,
                                                           encryptedMsgLength,
                                                           plainTextMsgPtr,
                                                           maxPlainTextLength);
                if(plaintextLength == olm_error())
                {
                    LOGE(" ## decryptMessageJni(): failure - olm_group_decrypt Msg=%s",(const char *)olm_inbound_group_session_last_error(sessionPtr));
                }
                else
                {
                    // update decrypted buffer size
                    plainTextMsgPtr[plaintextLength] = static_cast<char>('\0');

                    LOGD(" ## decryptMessageJni(): decrypted returnedLg=%lu plainTextMsgPtr=%s",plaintextLength, (char*)plainTextMsgPtr);
                    decryptedMsgRetValue = env->NewStringUTF((const char*)plainTextMsgPtr);
                }
            }
        }
    }

    // free alloc
    if(NULL != encryptedMsgPtr)
    {
        env->ReleaseStringUTFChars(aEncryptedMsg, encryptedMsgPtr);
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
* Serialize and encrypt session instance into a base64 string.<br>
* @param aKey key used to encrypt the serialized session data
* @param[out] aErrorMsg error message set if operation failed
* @return a base64 string if operation succeed, null otherwise
**/
JNIEXPORT jstring OLM_INBOUND_GROUP_SESSION_FUNC_DEF(serializeDataWithKeyJni)(JNIEnv *env, jobject thiz, jstring aKey, jobject aErrorMsg)
{
    jstring pickledDataRetValue = 0;
    jclass errorMsgJClass = 0;
    jmethodID errorMsgMethodId = 0;
    jstring errorJstring = 0;
    const char *keyPtr = NULL;
    void *pickledPtr = NULL;
    OlmInboundGroupSession* sessionPtr = NULL;

    LOGD("## inbound group session serializeDataWithKeyJni(): IN");

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
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
        size_t pickledLength = olm_pickle_inbound_group_session_length(sessionPtr);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        LOGD(" ## serializeDataWithKeyJni(): pickledLength=%lu keyLength=%lu",pickledLength, keyLength);
        LOGD(" ## serializeDataWithKeyJni(): key=%s",(char const *)keyPtr);

        if(NULL == (pickledPtr = (void*)malloc((pickledLength+1)*sizeof(uint8_t))))
        {
            LOGE(" ## serializeDataWithKeyJni(): failure - pickledPtr buffer OOM");
        }
        else
        {
            size_t result = olm_pickle_inbound_group_session(sessionPtr,
                                                             (void const *)keyPtr,
                                                              keyLength,
                                                              (void*)pickledPtr,
                                                              pickledLength);
            if(result == olm_error())
            {
                const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
                LOGE(" ## serializeDataWithKeyJni(): failure - olm_pickle_outbound_group_session() Msg=%s",errorMsgPtr);

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
                LOGD(" ## serializeDataWithKeyJni(): success - result=%lu pickled=%s", result, static_cast<char*>(pickledPtr));
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


JNIEXPORT jstring OLM_INBOUND_GROUP_SESSION_FUNC_DEF(initWithSerializedDataJni)(JNIEnv *env, jobject thiz, jstring aSerializedData, jstring aKey)
{
    OlmInboundGroupSession* sessionPtr = NULL;
    jstring errorMessageRetValue = 0;
    const char *keyPtr = NULL;
    const char *pickledPtr = NULL;

    LOGD("## initWithSerializedDataJni(): IN");

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
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
        LOGD(" ## initWithSerializedDataJni(): pickledLength=%lu keyLength=%lu",pickledLength, keyLength);
        LOGD(" ## initWithSerializedDataJni(): key=%s",(char const *)keyPtr);
        LOGD(" ## initWithSerializedDataJni(): pickled=%s",(char const *)pickledPtr);

        size_t result = olm_unpickle_inbound_group_session(sessionPtr,
                                                           (void const *)keyPtr,
                                                           keyLength,
                                                           (void*)pickledPtr,
                                                           pickledLength);
        if(result == olm_error())
        {
            const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
            LOGE(" ## initWithSerializedDataJni(): failure - olm_unpickle_inbound_group_session() Msg=%s",errorMsgPtr);
            errorMessageRetValue = env->NewStringUTF(errorMsgPtr);
        }
        else
        {
            LOGD(" ## initWithSerializedDataJni(): success - result=%lu ", result);
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
