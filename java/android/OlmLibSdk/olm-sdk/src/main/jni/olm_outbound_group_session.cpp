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

#include "olm_outbound_group_session.h"

using namespace AndroidOlmSdk;

/**
 * Release the session allocation made by initializeOutboundGroupSessionMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
  OlmOutboundGroupSession* sessionPtr = NULL;

  LOGD("## releaseSessionJni(): outbound group session");

  if(NULL == (sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
  {
      LOGE(" ## releaseSessionJni(): failure - invalid outbound group session instance");
  }
  else
  {
    LOGD(" ## releaseSessionJni(): sessionPtr=%p",sessionPtr);

    size_t retCode = olm_clear_outbound_group_session(sessionPtr);
    LOGD(" ## releaseSessionJni(): clear_outbound_group_session=%lu",retCode);

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
JNIEXPORT jlong OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(initNewSessionJni)(JNIEnv *env, jobject thiz)
{
    OlmOutboundGroupSession* sessionPtr = NULL;
    size_t sessionSize = 0;

    LOGD("## initNewSessionJni(): outbound group session IN");
    sessionSize = olm_outbound_group_session_size();

    if(0 == sessionSize)
    {
        LOGE(" ## initNewSessionJni(): failure - outbound group session size = 0");
    }
    else if(NULL != (sessionPtr=(OlmOutboundGroupSession*)malloc(sessionSize)))
    {
      sessionPtr = olm_outbound_group_session(sessionPtr);
      LOGD(" ## initNewSessionJni(): success - outbound group session size=%lu",sessionSize);
    }
    else
    {
      LOGE(" ## initNewSessionJni(): failure - outbound group session OOM");
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
    OlmOutboundGroupSession *sessionPtr = NULL;
    uint8_t *randomBuffPtr = NULL;

    LOGD("## initOutboundGroupSessionJni(): IN");

    if(NULL == (sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## initOutboundGroupSessionJni(): failure - invalid outbound group session instance");
    }
    else
    {
        // compute random buffer
        size_t randomLength = olm_init_outbound_group_session_random_length(sessionPtr);
        LOGW(" ## initOutboundGroupSessionJni(): randomLength=%lu",randomLength);
        if((0!=randomLength) && !setRandomInBuffer(&randomBuffPtr, randomLength))
        {
            LOGE(" ## initOutboundGroupSessionJni(): failure - random buffer init");
        }
        else
        {
            if(0==randomLength)
            {
                LOGW(" ## initOutboundGroupSessionJni(): random buffer is not required");
            }

            size_t sessionResult = olm_init_outbound_group_session(sessionPtr, randomBuffPtr, randomLength);
            if(sessionResult == olm_error()) {
                const char *errorMsgPtr = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## initOutboundGroupSessionJni(): failure - init outbound session creation  Msg=%s",errorMsgPtr);
            }
            else
            {
                retCode = ERROR_CODE_OK;
                LOGD(" ## initOutboundGroupSessionJni(): success - result=%lu", sessionResult);
            }
        }
      }

    if(NULL != randomBuffPtr)
    {
        free(randomBuffPtr);
    }

    return retCode;
}

/**
* Get a base64-encoded identifier for this outbound group session.
*/
JNIEXPORT jstring OLM_OUTBOUND_GROUP_SESSION_FUNC_DEF(sessionIdentifierJni)(JNIEnv *env, jobject thiz)
{
    OlmOutboundGroupSession *sessionPtr = NULL;
    uint8_t *sessionIdPtr = NULL;
    jstring returnValueStr=0;

    LOGD("## sessionIdentifierJni(): outbound group session IN");

    if(NULL == (sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## sessionIdentifierJni(): failure - invalid outbound group session instance");
    }
    else
    {
        // get the size to alloc
        size_t lengthSessionId = olm_outbound_group_session_id_length(sessionPtr);
        LOGD(" ## sessionIdentifierJni(): outbound group session lengthSessionId=%lu",lengthSessionId);

        if(NULL == (sessionIdPtr = (uint8_t*)malloc((lengthSessionId+1)*sizeof(uint8_t))))
        {
           LOGE(" ## sessionIdentifierJni(): failure - outbound identifier allocation OOM");
        }
        else
        {
            size_t result = olm_outbound_group_session_id(sessionPtr, sessionIdPtr, lengthSessionId);
            if (result == olm_error())
            {
                const char *errorMsgPtr = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## sessionIdentifierJni(): failure - outbound group session identifier failure Msg=%s",errorMsgPtr);
            }
            else
            {
                // update length
                sessionIdPtr[result] = static_cast<char>('\0');
                LOGD(" ## sessionIdentifierJni(): success - outbound group session identifier result=%lu sessionId=%s",result, (char*)sessionIdPtr);
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

    if(NULL == (sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
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
    OlmOutboundGroupSession *sessionPtr = NULL;
    uint8_t *sessionKeyPtr = NULL;
    jstring returnValueStr=0;

    LOGD("## sessionKeyJni(): outbound group session IN");

    if(NULL == (sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## sessionKeyJni(): failure - invalid outbound group session instance");
    }
    else
    {
        // get the size to alloc
        size_t sessionKeyLength = olm_outbound_group_session_key_length(sessionPtr);
        LOGD(" ## sessionKeyJni(): sessionKeyLength=%lu",sessionKeyLength);

        if(NULL == (sessionKeyPtr = (uint8_t*)malloc((sessionKeyLength+1)*sizeof(uint8_t))))
        {
           LOGE(" ## sessionKeyJni(): failure - session key allocation OOM");
        }
        else
        {
            size_t result = olm_outbound_group_session_key(sessionPtr, sessionKeyPtr, sessionKeyLength);
            if (result == olm_error())
            {
                const char *errorMsgPtr = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## sessionKeyJni(): failure - session key failure Msg=%s",errorMsgPtr);
            }
            else
            {
                // update length
                sessionKeyPtr[result] = static_cast<char>('\0');
                LOGD(" ## sessionKeyJni(): success - outbound group session key result=%lu sessionKey=%s",result, (char*)sessionKeyPtr);
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
    jstring encryptedMsgRetValue = 0;
    OlmOutboundGroupSession *sessionPtr = NULL;
    const char *clearMsgPtr = NULL;
    uint8_t *encryptedMsgPtr = NULL;

    LOGD("## encryptMessageJni(): IN");

    if(NULL == (sessionPtr = (OlmOutboundGroupSession*)getOutboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE(" ## encryptMessageJni(): failure - invalid outbound group session ptr=NULL");
    }
    else if(0 == aClearMsg)
    {
        LOGE(" ## encryptMessageJni(): failure - invalid clear message");
    }
    else if(0 == (clearMsgPtr = env->GetStringUTFChars(aClearMsg, 0)))
    {
        LOGE(" ## encryptMessageJni(): failure - clear message JNI allocation OOM");
    }
    else
    {
        // get clear message length
        size_t clearMsgLength = (size_t)env->GetStringUTFLength(aClearMsg);
        LOGD(" ## encryptMessageJni(): clearMsgLength=%lu",clearMsgLength);

        // compute max encrypted length
        size_t encryptedMsgLength = olm_group_encrypt_message_length(sessionPtr,clearMsgLength);
        if(NULL == (encryptedMsgPtr = (uint8_t*)malloc((encryptedMsgLength+1)*sizeof(uint8_t))))
        {
            LOGE(" ## encryptMessageJni(): failure - encryptedMsgPtr buffer OOM");
        }
        else
        {
            LOGD(" ## encryptMessageJni(): estimated encryptedMsgLength=%lu",encryptedMsgLength);

            size_t encryptedLength = olm_group_encrypt(sessionPtr,
                                                       (uint8_t*)clearMsgPtr,
                                                       clearMsgLength,
                                                       encryptedMsgPtr,
                                                       encryptedMsgLength);
            if(encryptedLength == olm_error())
            {
                const char *errorMsgPtr = olm_outbound_group_session_last_error(sessionPtr);
                LOGE(" ## encryptMessageJni(): failure - olm_group_encrypt Msg=%s",errorMsgPtr);
            }
            else
            {
                // update decrypted buffer size
                encryptedMsgPtr[encryptedLength] = static_cast<char>('\0');

                LOGD(" ## encryptMessageJni(): encrypted returnedLg=%lu plainTextMsgPtr=%s",encryptedLength, (char*)encryptedMsgPtr);
                encryptedMsgRetValue = env->NewStringUTF((const char*)encryptedMsgPtr);
            }
        }
      }

    // free alloc
    if(NULL != clearMsgPtr)
    {
        env->ReleaseStringUTFChars(aClearMsg, clearMsgPtr);
    }

    if(NULL != encryptedMsgPtr)
    {
        free(encryptedMsgPtr);
    }

    return encryptedMsgRetValue;
}



