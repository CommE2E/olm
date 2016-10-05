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


/**
* Init memory allocation for session creation.
* @return valid memory allocation, NULL otherwise
**/
OlmSession* initializeSessionMemory()
{
    OlmSession* sessionPtr = NULL;
    size_t sessionSize = olm_session_size();

    if(NULL != (sessionPtr=(OlmSession*)malloc(sessionSize)))
    { // init session object
      sessionPtr = olm_session(sessionPtr);
      LOGD("## initializeSessionMemory(): success - OLM session size=%lu",sessionSize);
    }
    else
    {
      LOGE("## initializeSessionMemory(): failure - OOM");
    }

    return sessionPtr;
}

JNIEXPORT void JNICALL Java_org_matrix_olm_OlmSession_releaseSessionJni(JNIEnv *env, jobject thiz)
{
  OlmSession* sessionPtr = NULL;

  if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
  {
      LOGE("## releaseSessionJni(): failure - invalid Session ptr=NULL");
  }
  else
  { // even if free(NULL) does not crash, a test is performed for debug purpose
    LOGD("## releaseSessionJni(): IN");
    free(sessionPtr);
    LOGD("## releaseSessionJni(): OUT");
  }
}

/**
* Initialize a new session and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmSession* => jlong) platform independent.
* @return the initialized OlmSession* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong JNICALL Java_org_matrix_olm_OlmSession_initNewSessionJni(JNIEnv *env, jobject thiz)
{
    OlmSession* sessionPtr = NULL;

    // init account memory allocation
    if(NULL == (sessionPtr = initializeSessionMemory()))
    {
        LOGE("## initNewSessionJni(): failure - init session OOM");
    }
    else
    {
       LOGD("## initNewSessionJni(): success - OLM session created");
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
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_initOutboundSessionJni(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aTheirOneTimeKey)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession* sessionPtr = NULL;
    OlmAccount* accountPtr = NULL;
    void *randomBuffPtr;
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
        if(NULL == (randomBuffPtr = (void*)malloc(randomSize*sizeof(void*))))
        {
            LOGE("## initOutboundSessionJni(): failure - random buffer OOM");
        }
        else
        {   // convert identity & one time keys to C strings
            const char* theirIdentityKeyPtr = NULL;
            const char* theirOneTimeKeyPtr = NULL;

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
                int theirIdentityKeyLength = env->GetStringUTFLength(aTheirIdentityKey);
                int theirOneTimeKeyLength  = env->GetStringUTFLength(aTheirOneTimeKey);
                LOGD("## initOutboundSessionJni(): identityKey=%s oneTimeKey=%s",theirIdentityKeyPtr,theirOneTimeKeyPtr);

                sessionResult = olm_create_outbound_session(sessionPtr, accountPtr, theirIdentityKeyPtr, theirIdentityKeyLength, theirOneTimeKeyPtr, theirOneTimeKeyLength, randomBuffPtr, randomSize);
                if(sessionResult == olm_error()) {
                    const char *errorMsgPtr = olm_session_last_error(sessionPtr);
                    LOGE("## initOutboundSessionJni(): failure - session creation  Msg=%s",errorMsgPtr);
                }
                else
                {
                    retCode = ERROR_CODE_OK;
                    LOGD("## initOutboundSessionJni(): success - result=%ld", sessionResult);
                }
            }

            // free local alloc
            free(randomBuffPtr);
            if(NULL!= theirIdentityKeyPtr)
            {
                env->ReleaseStringUTFChars(aTheirIdentityKey, theirIdentityKeyPtr);
            }
            if(NULL!= theirOneTimeKeyPtr)
            {
                env->ReleaseStringUTFChars(aTheirOneTimeKey, theirOneTimeKeyPtr);
            }
        }
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
 * @param aOneTimeKeyMsg PRE_KEY message TODO TBC
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
 */
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_initInboundSessionJni(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aOneTimeKeyMsg)
{
    jint retCode = ERROR_CODE_KO;
    OlmSession *sessionPtr = NULL;
    OlmAccount *accountPtr = NULL;
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
        LOGE("## initOutboundSessionJni(): failure - invalid message");
    }
    else
    {   // convert message to C strings
        const char *messagePtr = NULL;
        if(NULL == (messagePtr = env->GetStringUTFChars(aOneTimeKeyMsg, 0)))
        {
            LOGE("## initInboundSessionJni(): failure - message JNI allocation OOM");
        }
        else
        {
            int messageLength = env->GetStringUTFLength(aOneTimeKeyMsg);
            LOGD("## initInboundSessionJni(): message=%s messageLength=%d",messagePtr,messageLength);

            sessionResult = olm_create_inbound_session(sessionPtr, accountPtr, (void*)messagePtr , messageLength);
            if(sessionResult == olm_error()) {
                const char *errorMsgPtr = olm_session_last_error(sessionPtr);
                LOGE("## initInboundSessionJni(): failure - init inbound session creation  Msg=%s",errorMsgPtr);
            }
            else
            {
                retCode = ERROR_CODE_OK;
                LOGD("## initInboundSessionJni(): success - result=%ld", sessionResult);
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
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_initInboundSessionFromIdKeyJni(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg)
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
        LOGE("## initOutboundSessionJni(): failure - invalid one time key message");
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
        size_t messageLength = env->GetStringUTFLength(aOneTimeKeyMsg);
        size_t theirIdentityKeyLength = env->GetStringUTFLength(aTheirIdentityKey);

        LOGD("## initInboundSessionFromIdKeyJni(): message=%s messageLength=%lu",messagePtr,messageLength);

        sessionResult = olm_create_inbound_session_from(sessionPtr, accountPtr, theirIdentityKeyPtr, theirIdentityKeyLength, (void*)messagePtr , messageLength);
        if(sessionResult == olm_error()) {
            const char *errorMsgPtr = olm_session_last_error(sessionPtr);
            LOGE("## initInboundSessionFromIdKeyJni(): failure - init inbound session creation  Msg=%s",errorMsgPtr);
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## initInboundSessionFromIdKeyJni(): success - result=%ld", sessionResult);
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
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_matchesInboundSessionJni(JNIEnv *env, jobject thiz, jstring aOneTimeKeyMsg)
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
        size_t messageLength = env->GetStringUTFLength(aOneTimeKeyMsg);

        size_t matchResult = olm_matches_inbound_session(sessionPtr, (void*)messagePtr , messageLength);
        if(matchResult == olm_error()) {
            const char *errorMsgPtr = olm_session_last_error(sessionPtr);
            LOGE("## matchesInboundSessionJni(): failure - no match  Msg=%s",errorMsgPtr);
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## matchesInboundSessionJni(): success - result=%ld", matchResult);
        }
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
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_matchesInboundSessionFromIdKeyJni(JNIEnv *env, jobject thiz, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg)
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
        size_t identityKeyLength = env->GetStringUTFLength(aTheirIdentityKey);
        size_t messageLength = env->GetStringUTFLength(aOneTimeKeyMsg);

        size_t matchResult = olm_matches_inbound_session_from(sessionPtr, (void const *)theirIdentityKeyPtr, identityKeyLength, (void*)messagePtr , messageLength);
        if(matchResult == olm_error()) {
            const char *errorMsgPtr = olm_session_last_error(sessionPtr);
            LOGE("## matchesInboundSessionFromIdKeyJni(): failure - no match  Msg=%s",errorMsgPtr);
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## matchesInboundSessionFromIdKeyJni(): success - result=%lu", matchResult);
        }
    }

    return retCode;
}


/**
* Get the session identifier for this session.
* @return the session identifier if operation succeed, null otherwise
*/
JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmSession_getSessionIdentifierJni(JNIEnv *env, jobject thiz)
{
    OlmSession *sessionPtr = NULL;
    void *sessionIdPtr = NULL;
    jstring returnValueStr=0;

    // get the size to alloc to contain the id
    size_t lengthSessId = olm_session_id_length(sessionPtr);

    if(NULL == (sessionPtr = (OlmSession*)getSessionInstanceId(env,thiz)))
    {
        LOGE("## getSessionIdentifierJni(): failure - invalid Session ptr=NULL");
    }
    else if(NULL == (sessionIdPtr = (void*)malloc(lengthSessId*sizeof(void*))))
    {
       LOGE("## getSessionIdentifierJni(): failure - identifier allocation OOM");
    }
    else
    {
        size_t result = olm_session_id(sessionPtr, sessionIdPtr, lengthSessId);
        if (result == olm_error())
        {
            const char *errorMsgPtr = olm_session_last_error(sessionPtr);
            LOGE("## getSessionIdentifierJni(): failure - get session identifier failure Msg=%s",errorMsgPtr);
        }
        else
        {
            returnValueStr = env->NewStringUTF((const char*)sessionIdPtr);
        }
        free(sessionIdPtr);
    }

    return returnValueStr;
}

/**
* Read the account instance ID of the calling object (aJavaObject) passed in parameter.
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if read succeed, -1 otherwise.
**/
jlong getSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId=-1;
  jfieldID instanceIdField;
  jclass loaderClass;

  if(NULL!=aJniEnv)
  {
    if(0 != (loaderClass=aJniEnv->GetObjectClass(aJavaObject)))
    {
      if(0 != (instanceIdField=aJniEnv->GetFieldID(loaderClass, "mNativeOlmSessionId", "J")))
      {
        instanceId = aJniEnv->GetIntField(aJavaObject, instanceIdField);
        aJniEnv->DeleteLocalRef(loaderClass);
      }
      else
      {
        LOGD("## getSessionInstanceId() ERROR! GetFieldID=null");
      }
    }
    else
    {
      LOGD("## getSessionInstanceId() ERROR! GetObjectClass=null");
    }
  }
  else
  {
    LOGD("## getSessionInstanceId() ERROR! aJniEnv=NULL");
  }

  LOGD("## getSessionInstanceId() success - instanceId=%lld",instanceId);
  return instanceId;
}
