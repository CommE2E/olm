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


/**
 * Release the session allocation made by initializeInboundGroupSessionMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_INBOUND_GROUP_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz)
{
  OlmInboundGroupSession* sessionPtr = NULL;

  LOGD("## releaseSessionJni(): sessionPtr=%p",sessionPtr);

  if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
  {
      LOGE("## releaseSessionJni(): failure - invalid inbound group session instance");
  }
  else
  {
    size_t retCode = olm_clear_inbound_group_session(sessionPtr);
    LOGD("## releaseSessionJni(): clear_inbound_group_session=%lu",retCode);

    LOGD("## releaseSessionJni(): IN");
    free(sessionPtr);
    LOGD("## releaseSessionJni(): OUT");
  }
}

/**
* Initialize a new inbound group session and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmInboundGroupSession* => jlong) platform independent.
* @return the initialized OlmInboundGroupSession* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong OLM_INBOUND_GROUP_SESSION_FUNC_DEF(initNewSessionJni)(JNIEnv *env, jobject thiz)
{
    OlmInboundGroupSession* sessionPtr = NULL;
    size_t sessionSize = olm_inbound_group_session_size();

    if(0 == sessionSize)
    {
        LOGE("## initNewSessionJni(): failure - inbound group session size = 0");
    }
    else if(NULL != (sessionPtr=(OlmInboundGroupSession*)malloc(sessionSize)))
    {
      sessionPtr = olm_inbound_group_session(sessionPtr);
      LOGD("## initNewSessionJni(): success - inbound group session size=%lu",sessionSize);
    }
    else
    {
      LOGE("## initNewSessionJni(): failure - inbound group session OOM");
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

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE("## initInboundGroupSessionWithSessionKeyJni(): failure - invalid inbound group session instance");
    }
    else if(0 == aSessionKey)
    {
        LOGE("## initInboundGroupSessionWithSessionKeyJni(): failure - invalid aSessionKey");
    }
    else if(NULL == (sessionKeyPtr = (const uint8_t *)env->GetStringUTFChars(aSessionKey, 0)))
    {
        LOGE("## initInboundSessionFromIdKeyJni(): failure - session key JNI allocation OOM");
    }
    else
    {
        size_t sessionKeyLength = (size_t)env->GetStringUTFLength(aSessionKey);
        LOGD("## initInboundSessionFromIdKeyJni(): sessionKeyLength=%lu",sessionKeyLength);

        sessionResult = olm_init_inbound_group_session(sessionPtr, sessionKeyPtr, sessionKeyLength);
        if(sessionResult == olm_error()) {
            const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
            LOGE("## initInboundSessionFromIdKeyJni(): failure - init inbound session creation  Msg=%s",errorMsgPtr);
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## initInboundSessionFromIdKeyJni(): success - result=%lu", sessionResult);
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
        LOGE("## sessionIdentifierJni(): failure - invalid inbound group session instance");
    }
    else
    {
        // get the size to alloc
        size_t lengthSessionId = olm_inbound_group_session_id_length(sessionPtr);
        LOGD("## sessionIdentifierJni(): inbound group session lengthSessionId=%lu",lengthSessionId);

        if(NULL == (sessionIdPtr = (uint8_t*)malloc(lengthSessionId*sizeof(uint8_t))))
        {
           LOGE("## sessionIdentifierJni(): failure - inbound group session identifier allocation OOM");
        }
        else
        {
            size_t result = olm_inbound_group_session_id(sessionPtr, sessionIdPtr, lengthSessionId);
            if (result == olm_error())
            {
                const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
                LOGE("## sessionIdentifierJni(): failure - get inbound group session identifier failure Msg=%s",errorMsgPtr);
            }
            else
            {
                // update length
                sessionIdPtr[result] = static_cast<char>('\0');
                LOGD("## sessionIdentifierJni(): success - inbound group session result=%lu sessionId=%s",result, (char*)sessionIdPtr);
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

    LOGD("## decryptMessageJni(): IN");

    if(NULL == (sessionPtr = (OlmInboundGroupSession*)getInboundGroupSessionInstanceId(env,thiz)))
    {
        LOGE("##  decryptMessageJni(): failure - invalid inbound group session ptr=NULL");
    }
    else if(0 == aEncryptedMsg)
    {
        LOGE("##  decryptMessageJni(): failure - invalid encrypted message");
    }
    else if(0 == (encryptedMsgPtr = env->GetStringUTFChars(aEncryptedMsg, 0)))
    {
        LOGE("##  decryptMessageJni(): failure - encrypted message JNI allocation OOM");
    }
    else
    {
        // get encrypted message length
        size_t encryptedMsgLength = (size_t)env->GetStringUTFLength(aEncryptedMsg);

        // create a dedicated temp buffer to be used in next Olm API calls
        if(NULL == (tempEncryptedPtr = static_cast<uint8_t*>(malloc(encryptedMsgLength*sizeof(uint8_t)))))
        {
            LOGE("##  decryptMessageJni(): failure - tempEncryptedPtr allocation OOM");
        }
        else
        {
            memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
            LOGD("##  decryptMessageJni(): encryptedMsgLength=%lu encryptedMsg=%s",encryptedMsgLength,encryptedMsgPtr);

            // get max plaintext length
            size_t maxPlainTextLength = olm_group_decrypt_max_plaintext_length(sessionPtr,
                                                                               tempEncryptedPtr,
                                                                               encryptedMsgLength);
            if(maxPlainTextLength == olm_error())
            {
                const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
                LOGE("##  decryptMessageJni(): failure - olm_group_decrypt_max_plaintext_length Msg=%s",errorMsgPtr);
            }
            else
            {
                LOGD("##  decryptMessageJni(): maxPlaintextLength=%lu",maxPlainTextLength);

                // allocate output decrypted message
                plainTextMsgPtr = static_cast<uint8_t*>(malloc(maxPlainTextLength*sizeof(uint8_t)));

                // decrypt, but before reload encrypted buffer (previous one was destroyed)
                memcpy(tempEncryptedPtr, encryptedMsgPtr, encryptedMsgLength);
                size_t plaintextLength = olm_group_decrypt(sessionPtr,
                                                           tempEncryptedPtr,
                                                           encryptedMsgLength,
                                                           plainTextMsgPtr,
                                                           maxPlainTextLength);
                if(plaintextLength == olm_error())
                {
                    const char *errorMsgPtr = olm_inbound_group_session_last_error(sessionPtr);
                    LOGE("##  decryptMessageJni(): failure - olm_group_decrypt Msg=%s",errorMsgPtr);
                }
                else
                {
                    // update decrypted buffer size
                    plainTextMsgPtr[plaintextLength] = static_cast<char>('\0');

                    LOGD("##  decryptMessageJni(): decrypted returnedLg=%lu plainTextMsgPtr=%s",plaintextLength, (char*)plainTextMsgPtr);
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



