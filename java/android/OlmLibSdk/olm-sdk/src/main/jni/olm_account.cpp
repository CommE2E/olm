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

#include "olm_account.h"

using namespace AndroidOlmSdk;


/**
* Init memory allocation for account creation.
* @return valid memory allocation, NULL otherwise
**/
OlmAccount* initializeAccountMemory()
{
    OlmAccount* accountPtr = NULL;
    size_t accountSize = olm_account_size();

    if(NULL != (accountPtr=(OlmAccount*)malloc(accountSize)))
    { // init account object
      accountPtr = olm_account(accountPtr);
      LOGD("## initializeAccountMemory(): success - OLM account size=%lu",static_cast<long unsigned int>(accountSize));
    }
    else
    {
      LOGE("## initializeAccountMemory(): failure - OOM");
    }

    return accountPtr;
}


JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(createNewAccountJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## createNewAccountJni(): IN");
    OlmAccount* accountPtr = initializeAccountMemory();

    LOGD(" ## createNewAccountJni(): success - accountPtr=%p (jlong)(intptr_t)accountPtr=%lld",accountPtr,(jlong)(intptr_t)accountPtr);
    return (jlong)(intptr_t)accountPtr;
}


/**
 * Release the account allocation made by initializeAccountMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(releaseAccountJni)(JNIEnv *env, jobject thiz)
{
  OlmAccount* accountPtr = NULL;

  LOGD("## releaseAccountJni(): IN");

  if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
  {
      LOGE(" ## releaseAccountJni(): failure - invalid Account ptr=NULL");
  }
  else
  {
    LOGD(" ## releaseAccountJni(): accountPtr=%p",accountPtr);
    olm_clear_account(accountPtr);

    LOGD(" ## releaseAccountJni(): IN");
    // even if free(NULL) does not crash, logs are performed for debug purpose
    free(accountPtr);
    LOGD(" ## releaseAccountJni(): OUT");
  }
}

/**
* Initialize a new account and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmAccount* => jlong) platform independent.
* @return the initialized OlmAccount* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(initNewAccountJni)(JNIEnv *env, jobject thiz)
{
    OlmAccount *accountPtr = NULL;
    uint8_t *randomBuffPtr = NULL;
    size_t accountRetCode;
    size_t randomSize;

    // init account memory allocation
    if(NULL == (accountPtr = initializeAccountMemory()))
    {
        LOGE("## initNewAccount(): failure - init account OOM");
    }
    else
    {
        // get random buffer size
        randomSize = olm_create_account_random_length(accountPtr);
        LOGD("## initNewAccount(): randomSize=%lu", static_cast<long unsigned int>(randomSize));

        // allocate random buffer
        if((0!=randomSize) && !setRandomInBuffer(&randomBuffPtr, randomSize))
        {
            LOGE("## initNewAccount(): failure - random buffer init");
        }
        else
        {
            // create account
            accountRetCode = olm_create_account(accountPtr, (void*)randomBuffPtr, randomSize);
            if(accountRetCode == olm_error()) {
                LOGE("## initNewAccount(): failure - account creation failed Msg=%s", (const char *)olm_account_last_error(accountPtr));
             }

            LOGD("## initNewAccount(): success - OLM account created");
            LOGD("## initNewAccount(): success - accountPtr=%p (jlong)(intptr_t)accountPtr=%lld",accountPtr,(jlong)(intptr_t)accountPtr);
        }
    }

    if(NULL != randomBuffPtr)
    {
        free(randomBuffPtr);
    }

    return (jlong)(intptr_t)accountPtr;
}



// *********************************************************************
// ************************* IDENTITY KEYS API *************************
// *********************************************************************
/**
* Get identity keys: Ed25519 fingerprint key and Curve25519 identity key.<br>
* The keys are returned in the byte array.
* @return a valid byte array if operation succeed, null otherwise
**/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(identityKeysJni)(JNIEnv *env, jobject thiz)
{
    OlmAccount* accountPtr = NULL;
    size_t identityKeysLength;
    uint8_t *identityKeysBytesPtr;
    size_t keysResult;
    jbyteArray byteArrayRetValue = NULL;

    LOGD("## identityKeys(): accountPtr =%p",accountPtr);

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## identityKeys(): failure - invalid Account ptr=NULL");
    }
    else
    {   // identity keys allocation
        identityKeysLength = olm_account_identity_keys_length(accountPtr);
        if(NULL == (identityKeysBytesPtr=(uint8_t*)malloc(identityKeysLength*sizeof(uint8_t))))
        {
            LOGE("## identityKeys(): failure - identity keys array OOM");
        }
        else
        {   // retrieve key pairs in identityKeysBytesPtr
            keysResult = olm_account_identity_keys(accountPtr, identityKeysBytesPtr, identityKeysLength);
            if(keysResult == olm_error()) {
                LOGE("## identityKeys(): failure - error getting identity keys Msg=%s",(const char *)olm_account_last_error(accountPtr));
            }
            else
            {   // allocate the byte array to be returned to java
                if(NULL == (byteArrayRetValue=env->NewByteArray(identityKeysLength)))
                {
                    LOGE("## identityKeys(): failure - return byte array OOM");
                }
                else
                {
                    env->SetByteArrayRegion(byteArrayRetValue, 0/*offset*/, identityKeysLength, (const jbyte*)identityKeysBytesPtr);
                    LOGD("## identityKeys(): success - result=%lu", static_cast<long unsigned int>(keysResult));
                }
            }

            free(identityKeysBytesPtr);
        }
    }

    return byteArrayRetValue;
}

// *********************************************************************
// ************************* ONE TIME KEYS API *************************
// *********************************************************************
/**
 * Get the maximum number of "one time keys" the account can store.
 *
**/
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(maxOneTimeKeysJni)(JNIEnv *env, jobject thiz)
{
    OlmAccount* accountPtr = NULL;
    size_t maxKeys = -1;

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## maxOneTimeKey(): failure - invalid Account ptr=NULL");
    }
    else
    {
        maxKeys = olm_account_max_number_of_one_time_keys(accountPtr);
    }
    LOGD("## maxOneTimeKey(): Max keys=%lu", static_cast<long unsigned int>(maxKeys));

    return (jlong)maxKeys;
}

/**
 * Generate "one time keys".
 * @param aNumberOfKeys number of keys to generate
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
**/
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(generateOneTimeKeysJni)(JNIEnv *env, jobject thiz, jint aNumberOfKeys)
{
    OlmAccount *accountPtr = NULL;
    uint8_t *randomBufferPtr = NULL;
    jint retCode = ERROR_CODE_KO;
    size_t randomLength;
    size_t result;


    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## generateOneTimeKeysJni(): failure - invalid Account ptr");
    }
    else
    {   // keys memory allocation
        randomLength = olm_account_generate_one_time_keys_random_length(accountPtr, (size_t)aNumberOfKeys);
        LOGD("## generateOneTimeKeysJni(): randomLength=%lu", static_cast<long unsigned int>(randomLength));

        if((0!=randomLength) && !setRandomInBuffer(&randomBufferPtr, randomLength))
        {
            LOGE("## generateOneTimeKeysJni(): failure - random buffer init");
        }
        else
        {
            LOGD("## generateOneTimeKeysJni(): accountPtr =%p aNumberOfKeys=%d",accountPtr, aNumberOfKeys);

            // retrieve key pairs in keysBytesPtr
            result = olm_account_generate_one_time_keys(accountPtr, (size_t)aNumberOfKeys, (void*)randomBufferPtr, randomLength);
            if(result == olm_error()) {
                LOGE("## generateOneTimeKeysJni(): failure - error generating one time keys Msg=%s",(const char *)olm_account_last_error(accountPtr));
            }
            else
            {
                retCode = ERROR_CODE_OK;
                LOGD("## generateOneTimeKeysJni(): success - result=%lu", static_cast<long unsigned int>(result));
            }
        }
    }

    if(NULL != randomBufferPtr)
    {
        free(randomBufferPtr);
    }

    return retCode;
}

/**
 * Get "one time keys".<br>
 * Return the public parts of the unpublished "one time keys" for the account
 * @return a valid byte array if operation succeed, null otherwise
**/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(oneTimeKeysJni)(JNIEnv *env, jobject thiz)
{
    OlmAccount* accountPtr = NULL;
    size_t keysLength;
    uint8_t *keysBytesPtr;
    size_t keysResult;
    jbyteArray byteArrayRetValue = NULL;

    LOGD("## oneTimeKeysJni(): IN");

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## oneTimeKeysJni(): failure - invalid Account ptr");
    }
    else
    {   // keys memory allocation
        keysLength = olm_account_one_time_keys_length(accountPtr);
        if(NULL == (keysBytesPtr=(uint8_t *)malloc(keysLength*sizeof(uint8_t))))
        {
            LOGE("## oneTimeKeysJni(): failure - one time keys array OOM");
        }
        else
        {   // retrieve key pairs in keysBytesPtr
            keysResult = olm_account_one_time_keys(accountPtr, keysBytesPtr, keysLength);
            if(keysResult == olm_error()) {
                LOGE("## oneTimeKeysJni(): failure - error getting one time keys Msg=%s",(const char *)olm_account_last_error(accountPtr));
            }
            else
            {   // allocate the byte array to be returned to java
                if(NULL == (byteArrayRetValue=env->NewByteArray(keysLength)))
                {
                    LOGE("## oneTimeKeysJni(): failure - return byte array OOM");
                }
                else
                {
                    env->SetByteArrayRegion(byteArrayRetValue, 0/*offset*/, keysLength, (const jbyte*)keysBytesPtr);
                    LOGD("## oneTimeKeysJni(): success");
                }
            }

            free(keysBytesPtr);
        }
    }

    return byteArrayRetValue;
}

/**
 * Remove the "one time keys"  that the session used from the account.
 * Return the public parts of the unpublished "one time keys" for the account
 * @param aNativeOlmSessionId session instance
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_NO_MATCHING_ONE_TIME_KEYS if no matching keys, ERROR_CODE_KO otherwise
**/
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(removeOneTimeKeysForSessionJni)(JNIEnv *env, jobject thiz, jlong aNativeOlmSessionId)
{
    jint retCode = ERROR_CODE_KO;
    OlmAccount* accountPtr = NULL;
    OlmSession* sessionPtr = (OlmSession*)aNativeOlmSessionId;
    size_t result;

    if(NULL == sessionPtr)
    {
        LOGE("## removeOneTimeKeysForSessionJni(): failure - invalid session ptr");
    }
    else if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## removeOneTimeKeysForSessionJni(): failure - invalid account ptr");
    }
    else
    {
        result = olm_remove_one_time_keys(accountPtr, sessionPtr);
        if(result == olm_error())
        {   // the account doesn't have any matching "one time keys"..
            LOGW("## removeOneTimeKeysForSessionJni(): failure - removing one time keys Msg=%s",(const char *)olm_account_last_error(accountPtr));

            retCode = ERROR_CODE_NO_MATCHING_ONE_TIME_KEYS;
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## removeOneTimeKeysForSessionJni(): success");
        }
    }

    return retCode;
}

/**
 * Mark the current set of "one time keys" as being published.
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
**/
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(markOneTimeKeysAsPublishedJni)(JNIEnv *env, jobject thiz)
{
    jint retCode = ERROR_CODE_OK;
    OlmAccount* accountPtr = NULL;
    size_t result;

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## markOneTimeKeysAsPublishedJni(): failure - invalid account ptr");
        retCode = ERROR_CODE_KO;
    }
    else
    {
        result = olm_account_mark_keys_as_published(accountPtr);
        if(result == olm_error())
        {
            LOGW("## markOneTimeKeysAsPublishedJni(): failure - Msg=%s",(const char *)olm_account_last_error(accountPtr));
            retCode = ERROR_CODE_KO;
        }
        else
        {
            LOGD("## markOneTimeKeysAsPublishedJni(): success - retCode=%lu",static_cast<long unsigned int>(result));
        }
    }

    return retCode;
}

/**
 * Sign a message with the ed25519 key (fingerprint) for this account.<br>
 * The signed message is returned by the function.
 * @param aMessage message to sign
 * @return the signed message, null otherwise
**/
JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(signMessageJni)(JNIEnv *env, jobject thiz, jstring aMessage)
{
    OlmAccount* accountPtr = NULL;
    size_t signatureLength;
    void* signedMsgPtr;
    size_t resultSign;
    jstring signedMsgRetValue = NULL;

    if(NULL == aMessage)
    {
        LOGE("## signMessageJni(): failure - invalid aMessage param");
    }
    else if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## signMessageJni(): failure - invalid account ptr");
    }
    else
    {
        // convert message from JAVA to C string
        const char* messageToSign = env->GetStringUTFChars(aMessage, 0);
        if(NULL == messageToSign)
        {
            LOGE("## signMessageJni(): failure - message JNI allocation OOM");
        }
        else
        {
            int messageLength = env->GetStringUTFLength(aMessage);

            // signature memory allocation
            signatureLength = olm_account_signature_length(accountPtr);
            if(NULL == (signedMsgPtr = (void*)malloc((signatureLength+1)*sizeof(uint8_t))))
            {
                LOGE("## signMessageJni(): failure - signature allocation OOM");
            }
            else
            {   // sign message
                resultSign = olm_account_sign(accountPtr,
                                             (void*)messageToSign,
                                             (size_t)messageLength,
                                             signedMsgPtr,
                                             signatureLength);
                if(resultSign == olm_error())
                {
                    LOGE("## signMessageJni(): failure - error signing message Msg=%s",(const char *)olm_account_last_error(accountPtr));
                }
                else
                {
                    // info: signatureLength is always equal to resultSign
                    (static_cast<char*>(signedMsgPtr))[signatureLength] = static_cast<char>('\0');
                    // convert to jstring
                    signedMsgRetValue = env->NewStringUTF((const char*)signedMsgPtr); // UTF8
                    LOGD("## signMessageJni(): success - retCode=%lu signatureLength=%lu", static_cast<long unsigned int>(resultSign), static_cast<long unsigned int>(signatureLength));
                }

                free(signedMsgPtr);
            }

            // release messageToSign
            env->ReleaseStringUTFChars(aMessage, messageToSign);
        }
    }

    return signedMsgRetValue;
}


JNIEXPORT jstring OLM_MANAGER_FUNC_DEF(getOlmLibVersionJni)(JNIEnv* env, jobject thiz)
{
  uint8_t majorVer=0, minorVer=0, patchVer=0;
  jstring returnValueStr=0;
  char buff[150];

  olm_get_library_version(&majorVer, &minorVer, &patchVer);
  LOGD("## getOlmLibVersionJni(): Major=%d Minor=%d Patch=%d", majorVer, minorVer, patchVer);

  snprintf(buff, sizeof(buff), "%d.%d.%d", majorVer, minorVer, patchVer);
  returnValueStr = env->NewStringUTF((const char*)buff);

  return returnValueStr;
}

/**
* Serialize and encrypt account instance into a base64 string.<br>
* @param aKey key used to encrypt the serialized account data
* @param[out] aErrorMsg error message set if operation failed
* @return a base64 string if operation succeed, null otherwise
**/
JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(serializeDataWithKeyJni)(JNIEnv *env, jobject thiz, jstring aKey, jobject aErrorMsg)
{
    /*jstring pickledDataRetValue = serializeDataWithKey(env,thiz,
                                                          aKey,
                                                          aErrorMsg,
                                                          olm_pickle_account_length,
                                                          olm_pickle_account,
                                                          olm_account_last_error);
    return pickledDataRetValue;*/

    jstring pickledDataRetValue = 0;
    jclass errorMsgJClass = 0;
    jmethodID errorMsgMethodId = 0;
    jstring errorJstring = 0;
    const char *keyPtr = NULL;
    void *pickledPtr = NULL;
    OlmAccount* accountPtr = NULL;

    LOGD("## serializeDataWithKeyJni(): IN");

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE(" ## serializeDataWithKeyJni(): failure - invalid account ptr");
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
        size_t pickledLength = olm_pickle_account_length(accountPtr);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        LOGD(" ## serializeDataWithKeyJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## serializeDataWithKeyJni(): key=%s",(char const *)keyPtr);

        if(NULL == (pickledPtr = (void*)malloc((pickledLength+1)*sizeof(uint8_t))))
        {
            LOGE(" ## serializeDataWithKeyJni(): failure - pickledPtr buffer OOM");
        }
        else
        {
            size_t result = olm_pickle_account(accountPtr,
                                               (void const *)keyPtr,
                                               keyLength,
                                               (void*)pickledPtr,
                                               pickledLength);
            if(result == olm_error())
            {
                const char *errorMsgPtr = olm_account_last_error(accountPtr);
                LOGE(" ## serializeDataWithKeyJni(): failure - olm_pickle_account() Msg=%s",errorMsgPtr);

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


JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(initWithSerializedDataJni)(JNIEnv *env, jobject thiz, jstring aSerializedData, jstring aKey)
{
    OlmAccount* accountPtr = NULL;
    jstring errorMessageRetValue = 0;
    const char *keyPtr = NULL;
    const char *pickledPtr = NULL;

    LOGD("## initWithSerializedDataJni(): IN");

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    //if(NULL == (accountPtr = initializeAccountMemory()))
    {
        LOGE(" ## initWithSerializedDataJni(): failure - account failure OOM");
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

        size_t result = olm_unpickle_account(accountPtr,
                                             (void const *)keyPtr,
                                             keyLength,
                                             (void*)pickledPtr,
                                             pickledLength);
        if(result == olm_error())
        {
            const char *errorMsgPtr = olm_account_last_error(accountPtr);
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