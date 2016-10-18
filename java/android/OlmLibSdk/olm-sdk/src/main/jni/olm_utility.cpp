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

#include "olm_utility.h"

using namespace AndroidOlmSdk;

OlmUtility* initializeUtilityMemory()
{
    OlmUtility* utilityPtr = NULL;
    size_t utilitySize = olm_utility_size();

    if(NULL != (utilityPtr=(OlmUtility*)malloc(utilitySize)))
    {
      utilityPtr = olm_utility(utilityPtr);
      LOGD("## initializeUtilityMemory(): success - OLM utility size=%lu",utilitySize);
    }
    else
    {
      LOGE("## initializeUtilityMemory(): failure - OOM");
    }

    return utilityPtr;
}


JNIEXPORT jlong OLM_UTILITY_FUNC_DEF(initUtilityJni)(JNIEnv *env, jobject thiz)
{
    OlmUtility* utilityPtr = NULL;

    LOGD("## initUtilityJni(): IN");

    // init account memory allocation
    if(NULL == (utilityPtr = initializeUtilityMemory()))
    {
        LOGE(" ## initUtilityJni(): failure - init OOM");
    }
    else
    {
       LOGD(" ## initUtilityJni(): success");
    }

    return (jlong)(intptr_t)utilityPtr;
}


JNIEXPORT void OLM_UTILITY_FUNC_DEF(releaseUtilityJni)(JNIEnv *env, jobject thiz)
{
  OlmUtility* utilityPtr = NULL;

  if(NULL == (utilityPtr = (OlmUtility*)getUtilityInstanceId(env,thiz)))
  {
      LOGE("## releaseUtilityJni(): failure - utility ptr=NULL");
  }
  else
  {
    olm_clear_utility(utilityPtr);
    free(utilityPtr);
  }
}


/**
 * Verify an ed25519 signature.
 * If the key was too small then the message will be "OLM.INVALID_BASE64".
 * If the signature was invalid then the message will be "OLM.BAD_MESSAGE_MAC".
 *
 * @param aSignature the base64-encoded message signature to be checked.
 * @param aKey the ed25519 key (fingerprint key)
 * @param aMessage the message which was signed
 * @return 0 if validation succeed, an error message string if operation failed
 */
JNIEXPORT jstring OLM_UTILITY_FUNC_DEF(verifyEd25519SignatureJni)(JNIEnv *env, jobject thiz, jstring aSignature, jstring aKey, jstring aMessage)
{
    jstring errorMessageRetValue = 0;
    OlmUtility* utilityPtr = NULL;
    const char* signaturePtr = NULL;
    const char* keyPtr = NULL;
    const char* messagePtr = NULL;

    LOGD("## verifyEd25519SignatureJni(): IN");

    if(NULL == (utilityPtr = (OlmUtility*)getUtilityInstanceId(env,thiz)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - invalid utility ptr=NULL");
    }
    else if((0 == aSignature) || (0 == aKey) || (0 == aMessage))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - invalid input parameters ");
    }
    else if(0 == (signaturePtr = env->GetStringUTFChars(aSignature, 0)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - signature JNI allocation OOM");
    }
    else if(0 == (keyPtr = env->GetStringUTFChars(aKey, 0)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - key JNI allocation OOM");
    }
    else if(0 == (messagePtr = env->GetStringUTFChars(aMessage, 0)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - message JNI allocation OOM");
    }
    else
    {
        size_t signatureLength = (size_t)env->GetStringUTFLength(aSignature);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        size_t messageLength = (size_t)env->GetStringUTFLength(aMessage);
        LOGD(" ## verifyEd25519SignatureJni(): signatureLength=%lu keyLength=%lu messageLength=%lu",signatureLength,keyLength,messageLength);

        size_t result = olm_ed25519_verify(utilityPtr,
                                           (void const *)keyPtr,
                                           keyLength,
                                           (void const *)messagePtr,
                                           messageLength,
                                           (void*)signaturePtr,
                                           signatureLength);
        if(result == olm_error()) {
            const char *errorMsgPtr = olm_utility_last_error(utilityPtr);
            errorMessageRetValue = env->NewStringUTF(errorMsgPtr);
            LOGE("## verifyEd25519SignatureJni(): failure - session creation  Msg=%s",errorMsgPtr);
        }
        else
        {
            LOGD("## verifyEd25519SignatureJni(): success - result=%ld", result);
        }
    }

    // free alloc
    if(NULL != signaturePtr)
    {
     env->ReleaseStringUTFChars(aSignature, signaturePtr);
    }

    if(NULL != keyPtr)
    {
     env->ReleaseStringUTFChars(aKey, keyPtr);
    }

    if(NULL != messagePtr)
    {
     env->ReleaseStringUTFChars(aMessage, messagePtr);
    }

    return errorMessageRetValue;
}

/**
* Compute the digest (SHA 256) for the message passed in parameter.<br>
* The digest value is the function return value.
* @param aMessage
* @return digest of the message if operation succeed, null otherwise
**/
JNIEXPORT jstring OLM_UTILITY_FUNC_DEF(sha256Jni)(JNIEnv *env, jobject thiz, jstring aMessageToHash)
{
    jstring sha256RetValue = 0;
    OlmUtility* utilityPtr = NULL;
    const char* messagePtr = NULL;
    void *hashValuePtr = NULL;

    LOGD("## sha256Jni(): IN");

    if(NULL == (utilityPtr = (OlmUtility*)getUtilityInstanceId(env,thiz)))
    {
        LOGE(" ## sha256Jni(): failure - invalid utility ptr=NULL");
    }
    else if(0 == aMessageToHash)
    {
        LOGE(" ## sha256Jni(): failure - invalid message parameters ");
    }
    else if(0 == (messagePtr = env->GetStringUTFChars(aMessageToHash, 0)))
    {
        LOGE(" ## sha256Jni(): failure - message JNI allocation OOM");
    }
    else
    {
        // get lengths
        size_t messageLength = (size_t)env->GetStringUTFLength(aMessageToHash);
        size_t hashLength = olm_sha256_length(utilityPtr);

        if(NULL == (hashValuePtr = static_cast<void*>(malloc((hashLength+1)*sizeof(uint8_t)))))
        {
            LOGE("## sha256Jni(): failure - hash value allocation OOM");
        }
        else
        {
            size_t result = olm_sha256(utilityPtr,
                                       (void const *)messagePtr,
                                       messageLength,
                                       (void *)hashValuePtr,
                                       hashLength);
            if(result == olm_error())
            {
                const char *errorMsgPtr = olm_utility_last_error(utilityPtr);
                LOGE("## sha256Jni(): failure - hash creation Msg=%s",errorMsgPtr);
            }
            else
            {
                // update length
                (static_cast<char*>(hashValuePtr))[result] = static_cast<char>('\0');

                LOGD("## sha256Jni(): success - result=%lu hashValue=%s",result, (char*)hashValuePtr);
                sha256RetValue = env->NewStringUTF((const char*)hashValuePtr);
            }
        }

    }

    if(NULL != hashValuePtr)
    {
        free(hashValuePtr);
    }

    if(NULL != messagePtr)
    {
        env->ReleaseStringUTFChars(aMessageToHash, messagePtr);
    }

    return sha256RetValue;
}