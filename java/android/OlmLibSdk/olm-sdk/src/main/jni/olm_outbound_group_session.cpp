/*
 * Copyright 2016 OpenMarket Ltd
 * Copyright 2016 Vector Creations Ltd
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

#include "olm_outbound_group_session.h"

using namespace AndroidOlmSdk;

/**
 * Release the session allocation made by initializeOutboundGroupSessionMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releaseSessionJni(): OutBound group session IN");

    OlmOutboundGroupSession* sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);

    if (!sessionPtr)
    {
        LOGE(" ## releaseSessionJni(): failure - invalid outbound group session instance");
    }
    else
    {
        LOGD(" ## releaseSessionJni(): sessionPtr=%p",sessionPtr);

#ifdef ENABLE_JNI_LOG
        size_t retCode = olm_clear_outbound_group_session(sessionPtr);
        LOGD(" ## releaseSessionJni(): clear_outbound_group_session=%lu",static_cast<long unsigned int>(retCode));
#else
        olm_clear_outbound_group_session(sessionPtr);
#endif

        LOGD(" ## releaseSessionJni(): free IN");
        free(sessionPtr);
        LOGD(" ## releaseSessionJni(): free OUT");
    }
}

/**
* Initialize a new outbound group session and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmOutboundGroupSession* => jlong) platform independent.
* @return the initialized OlmOutboundGroupSession* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(createNewSessionJni)(JNIEnv *env, jobject thiz)
{
    OlmOutboundGroupSession* sessionPtr = NULL;
    size_t sessionSize = 0;

    LOGD("## createNewSessionJni(): outbound group session IN");
    sessionSize = olm_outbound_group_session_size();

    if (0 == sessionSize)
    {
        LOGE(" ## createNewSessionJni(): failure - outbound group session size = 0");
    }
    else if (!(sessionPtr = (OlmOutboundGroupSession*)malloc(sessionSize)))
    {
        sessionPtr = olm_outbound_group_session(sessionPtr);
        LOGD(" ## createNewSessionJni(): success - outbound group session size=%lu",static_cast<long unsigned int>(sessionSize));
    }
    else
    {
        LOGE(" ## createNewSessionJni(): failure - outbound group session OOM");
    }

    return (jlong)(intptr_t)sessionPtr;
}

/**
 * Start a new outbound session.<br>
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(initOutboundGroupSessionJni)(JNIEnv *env, jobject thiz)
{
    jint retCode = ERROR_CODE_KO;

    LOGD("## initOutboundGroupSessionJni(): IN");

    OlmOutboundGroupSession *sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);

    if (!sessionPtr)
    {
        LOGE(" ## initOutboundGroupSessionJni(): failure - invalid outbound group session instance");
    }
    else
    {
        // compute random buffer
        size_t randomLength = olm_init_outbound_group_session_random_length(sessionPtr);
        uint8_t *randomBuffPtr = NULL;

        LOGW(" ## initOutboundGroupSessionJni(): randomLength=%lu",static_cast<long unsigned int>(randomLength));

        if ((0 != randomLength) && !setRandomInBuffer(env, &randomBuffPtr, randomLength))
        {
            LOGE(" ## initOutboundGroupSessionJni(): failure - random buffer init");
        }
        else
        {
            if (0 == randomLength)
            {
                LOGW(" ## initOutboundGroupSessionJni(): random buffer is not required");
            }

            size_t sessionResult = olm_init_outbound_group_session(sessionPtr, randomBuffPtr, randomLength);

            if (sessionResult == olm_error()) {
                LOGE(" ## initOutboundGroupSessionJni(): failure - init outbound session creation  Msg=%s",(const char *)olm_outbound_group_session_last_error(sessionPtr));
            }
            else
            {
                retCode = ERROR_CODE_OK;
                LOGD(" ## initOutboundGroupSessionJni(): success - result=%lu", static_cast<long unsigned int>(sessionResult));
            }

            free(randomBuffPtr);
        }
    }

    return retCode;
}

/**
* Get a base64-encoded identifier for this outbound group session.
*/
JNIEXPORT jstring OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionIdentifierJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## sessionIdentifierJni(): outbound group session IN");
    OlmOutboundGroupSession *sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);
    jstring returnValueStr=0;

    if (!sessionPtr)
    {
        LOGE(" ## sessionIdentifierJni(): failure - invalid outbound group session instance");
    }
    else
    {
        // get the size to alloc
        size_t lengthSessionId = olm_outbound_group_session_id_length(sessionPtr);
        LOGD(" ## sessionIdentifierJni(): outbound group session lengthSessionId=%lu",static_cast<long unsigned int>(lengthSessionId));

        uint8_t *sessionIdPtr =  (uint8_t*)malloc((lengthSessionId+1)*sizeof(uint8_t));

        if (!sessionIdPtr)
        {
           LOGE(" ## sessionIdentifierJni(): failure - outbound identifier allocation OOM");
        }
        else
        {
            size_t result = olm_outbound_group_session_id(sessionPtr, sessionIdPtr, lengthSessionId);

            if (result == olm_error())
            {
                LOGE(" ## sessionIdentifierJni(): failure - outbound group session identifier failure Msg=%s",reinterpret_cast<const char*>(olm_outbound_group_session_last_error(sessionPtr)));
            }
            else
            {
                // update length
                sessionIdPtr[result] = static_cast<char>('\0');
                LOGD(" ## sessionIdentifierJni(): success - outbound group session identifier result=%lu sessionId=%s",static_cast<long unsigned int>(result), reinterpret_cast<char*>(sessionIdPtr));
                returnValueStr = env->NewStringUTF((const char*)sessionIdPtr);
            }

            // free alloc
            free(sessionIdPtr);
        }
    }

    return returnValueStr;
}


/**
* Get the current message index for this session.<br>
* Each message is sent with an increasing index, this
* method returns the index for the next message.
* @return current session index
*/
JNIEXPORT jint OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(messageIndexJni)(JNIEnv *env, jobject thiz)
{
    OlmOutboundGroupSession *sessionPtr = NULL;
    jint indexRetValue = 0;

    LOGD("## messageIndexJni(): IN");

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## messageIndexJni(): failure - invalid outbound group session instance");
    }
    else
    {
        indexRetValue = static_cast<jint>(olm_outbound_group_session_message_index(sessionPtr));
    }

    LOGD(" ## messageIndexJni(): success - index=%d",indexRetValue);

    return indexRetValue;
}

/**
* Get the base64-encoded current ratchet key for this session.<br>
*/
JNIEXPORT jstring OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionKeyJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## sessionKeyJni(): outbound group session IN");
    OlmOutboundGroupSession *sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz);
    jstring returnValueStr = 0;

    if (!sessionPtr)
    {
        LOGE(" ## sessionKeyJni(): failure - invalid outbound group session instance");
    }
    else
    {
        // get the size to alloc
        size_t sessionKeyLength = olm_outbound_group_session_key_length(sessionPtr);
        LOGD(" ## sessionKeyJni(): sessionKeyLength=%lu",static_cast<long unsigned int>(sessionKeyLength));

        uint8_t *sessionKeyPtr = (uint8_t*)malloc((sessionKeyLength+1)*sizeof(uint8_t));

        if (!sessionKeyPtr)
        {
           LOGE(" ## sessionKeyJni(): failure - session key allocation OOM");
        }
        else
        {
            size_t result = olm_outbound_group_session_key(sessionPtr, sessionKeyPtr, sessionKeyLength);

            if (result == olm_error())
            {
                LOGE(" ## sessionKeyJni(): failure - session key failure Msg=%s",(const char *)olm_outbound_group_session_last_error(sessionPtr));
            }
            else
            {
                // update length
                sessionKeyPtr[result] = static_cast<char>('\0');
                LOGD(" ## sessionKeyJni(): success - outbound group session key result=%lu sessionKey=%s",static_cast<long unsigned int>(result), reinterpret_cast<char*>(sessionKeyPtr));
                returnValueStr = env->NewStringUTF((const char*)sessionKeyPtr);
            }

            // free alloc
            free(sessionKeyPtr);
        }
    }

    return returnValueStr;
}


JNIEXPORT jstring OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jstring aClearMsg)
{
    LOGD("## encryptMessageJni(): IN");

    jstring encryptedMsgRetValue = 0;
    OlmOutboundGroupSession *sessionPtr = NULL;
    const char *clearMsgPtr = NULL;

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## encryptMessageJni(): failure - invalid outbound group session ptr=NULL");
    }
    else if (!aClearMsg)
    {
        LOGE(" ## encryptMessageJni(): failure - invalid clear message");
    }
    else if (!(clearMsgPtr = env->GetStringUTFChars(aClearMsg, 0)))
    {
        LOGE(" ## encryptMessageJni(): failure - clear message JNI allocation OOM");
    }
    else
    {
        // get clear message length
        size_t clearMsgLength = (size_t)env->GetStringUTFLength(aClearMsg);
        LOGD(" ## encryptMessageJni(): clearMsgLength=%lu",static_cast<long unsigned int>(clearMsgLength));

        // compute max encrypted length
        size_t encryptedMsgLength = olm_group_encrypt_message_length(sessionPtr,clearMsgLength);
        uint8_t *encryptedMsgPtr = (uint8_t*)malloc((encryptedMsgLength+1)*sizeof(uint8_t));

        if (!encryptedMsgPtr)
        {
            LOGE(" ## encryptMessageJni(): failure - encryptedMsgPtr buffer OOM");
        }
        else
        {
            LOGD(" ## encryptMessageJni(): estimated encryptedMsgLength=%lu",static_cast<long unsigned int>(encryptedMsgLength));

            size_t encryptedLength = olm_group_encrypt(sessionPtr,
                                                       (uint8_t*)clearMsgPtr,
                                                       clearMsgLength,
                                                       encryptedMsgPtr,
                                                       encryptedMsgLength);
            if (encryptedLength == olm_error())
            {
                LOGE(" ## encryptMessageJni(): failure - olm_group_encrypt Msg=%s",(const char *)olm_outbound_group_session_last_error(sessionPtr));
            }
            else
            {
                // update decrypted buffer size
                encryptedMsgPtr[encryptedLength] = static_cast<char>('\0');

                LOGD(" ## encryptMessageJni(): encrypted returnedLg=%lu plainTextMsgPtr=%s",static_cast<long unsigned int>(encryptedLength), reinterpret_cast<char*>(encryptedMsgPtr));
                encryptedMsgRetValue = env->NewStringUTF((const char*)encryptedMsgPtr);
            }

            free(encryptedMsgPtr);
         }
      }

    // free alloc
    if (clearMsgPtr)
    {
        env->ReleaseStringUTFChars(aClearMsg, clearMsgPtr);
    }

    return encryptedMsgRetValue;
}


/**
* Serialize and encrypt session instance into a base64 string.<br>
* @param aKey key used to encrypt the serialized session data
* @param[out] aErrorMsg error message set if operation failed
* @return a base64 string if operation succeed, null otherwise
**/
JNIEXPORT jstring OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(serializeDataWithKeyJni)(JNIEnv *env, jobject thiz, jstring aKey, jobject aErrorMsg)
{
    jstring pickledDataRetValue = 0;
    jclass errorMsgJClass = 0;
    jmethodID errorMsgMethodId = 0;
    jstring errorJstring = 0;
    const char *keyPtr = NULL;
    OlmOutboundGroupSession* sessionPtr = NULL;

    LOGD("## outbound group session serializeDataWithKeyJni(): IN");

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid session ptr");
    }
    else if (!aKey)
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid key");
    }
    else if (!aErrorMsg)
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid error object");
    }
    else if (!(errorMsgJClass = env->GetObjectClass(aErrorMsg)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - unable to get error class");
    }
    else if (!(errorMsgMethodId = env->GetMethodID(errorMsgJClass, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;")))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - unable to get error method ID");
    }
    else if (!(keyPtr = env->GetStringUTFChars(aKey, 0)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - keyPtr JNI allocation OOM");
    }
    else
    {
        size_t pickledLength = olm_pickle_outbound_group_session_length(sessionPtr);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        LOGD(" ## serializeDataWithKeyJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## serializeDataWithKeyJni(): key=%s",(char const *)keyPtr);

        void *pickledPtr = malloc((pickledLength+1)*sizeof(uint8_t));

        if(!pickledPtr)
        {
            LOGE(" ## serializeDataWithKeyJni(): failure - pickledPtr buffer OOM");
        }
        else
        {
            size_t result = olm_pickle_outbound_group_session(sessionPtr,
                                                             (void const *)keyPtr,
                                                              keyLength,
                                                              (void*)pickledPtr,
                                                              pickledLength);
            if (result == olm_error())
            {
                const char *errorMsgPtr = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## serializeDataWithKeyJni(): failure - olm_pickle_outbound_group_session() Msg=%s",errorMsgPtr);

                if (!(errorJstring = env->NewStringUTF(errorMsgPtr)))
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

        free(pickledPtr);
    }

    // free alloc
    if (keyPtr)
    {
        env->ReleaseStringUTFChars(aKey, keyPtr);
    }

    return pickledDataRetValue;
}


JNIEXPORT jstring OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(initWithSerializedDataJni)(JNIEnv *env, jobject thiz, jstring aSerializedData, jstring aKey)
{
    OlmOutboundGroupSession* sessionPtr = NULL;
    jstring errorMessageRetValue = 0;
    const char *keyPtr = NULL;
    const char *pickledPtr = NULL;

    LOGD("## initWithSerializedDataJni(): IN");

    if (!(sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## initWithSerializedDataJni(): failure - session failure OOM");
    }
    else if (!aKey)
    {
        LOGE(" ## initWithSerializedDataJni(): failure - invalid key");
    }
    else if (!aSerializedData)
    {
        LOGE(" ## initWithSerializedDataJni(): failure - serialized data");
    }
    else if (!(keyPtr = env->GetStringUTFChars(aKey, 0)))
    {
        LOGE(" ## initWithSerializedDataJni(): failure - keyPtr JNI allocation OOM");
    }
    else if (!(pickledPtr = env->GetStringUTFChars(aSerializedData, 0)))
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

        size_t result = olm_unpickle_outbound_group_session(sessionPtr,
                                                            (void const *)keyPtr,
                                                            keyLength,
                                                            (void*)pickledPtr,
                                                            pickledLength);
        if (result == olm_error())
        {
            const char *errorMsgPtr = olm_outbound_group_session_last_error(sessionPtr);
            LOGE(" ## initWithSerializedDataJni(): failure - olm_unpickle_outbound_group_session() Msg=%s",errorMsgPtr);
            errorMessageRetValue = env->NewStringUTF(errorMsgPtr);
        }
        else
        {
            LOGD(" ## initWithSerializedDataJni(): success - result=%lu ", static_cast<long unsigned int>(result));
        }
    }

    // free alloc
    if (keyPtr)
    {
        env->ReleaseStringUTFChars(aKey, keyPtr);
    }

    if (pickledPtr)
    {
        env->ReleaseStringUTFChars(aSerializedData, pickledPtr);
    }

    return errorMessageRetValue;
}

