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
#include "olm_utility.h"

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
      LOGD("## initializeAccountMemory(): success - OLM account size=%lu",accountSize);
    }
    else
    {
      LOGE("## initializeAccountMemory(): failure - OOM");
    }

    return accountPtr;
}

/**
 * Release the account allocation made by initializeAccountMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 *
 */
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(releaseAccountJni)(JNIEnv *env, jobject thiz)
{
  OlmAccount* accountPtr = NULL;

  LOGD("## releaseAccountJni(): accountPtr=%p",accountPtr);

  if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
  {
      LOGE("## releaseAccountJni(): failure - invalid Account ptr=NULL");
  }
  else
  {
    olm_clear_account(accountPtr);

    LOGD("## releaseAccountJni(): IN");
    // even if free(NULL) does not crash, logs are performed for debug purpose
    free(accountPtr);
    LOGD("## releaseAccountJni(): OUT");
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
        // allocate random buffer
        randomSize = olm_create_account_random_length(accountPtr);
        if(!setRandomInBuffer(&randomBuffPtr, randomSize))
        {
            LOGE("## initNewAccount(): failure - random buffer init");
        }
        else
        {
            // create account
            accountRetCode = olm_create_account(accountPtr, (void*)randomBuffPtr, randomSize);
            if(accountRetCode == olm_error()) {
                const char *errorMsgPtr = olm_account_last_error(accountPtr);
                LOGE("## initNewAccount(): failure - account creation failed Msg=%s", errorMsgPtr);
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
                const char *errorMsgPtr = olm_account_last_error(accountPtr);
                LOGE("## identityKeys(): failure - error getting identity keys Msg=%s",errorMsgPtr);
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
                    LOGD("## identityKeys(): success - result=%ld", keysResult);
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
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(maxOneTimeKeys)(JNIEnv *env, jobject thiz)
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
    LOGD("## maxOneTimeKey(): Max keys=%ld", maxKeys);

    return (jlong)maxKeys;
}

/**
 * Generate "one time keys".
 * @param aNumberOfKeys number of keys to generate
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
**/
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(generateOneTimeKeys)(JNIEnv *env, jobject thiz, jint aNumberOfKeys)
{
    OlmAccount *accountPtr = NULL;
    uint8_t *randomBufferPtr = NULL;
    jint retCode = ERROR_CODE_KO;
    size_t randomLength;
    size_t result;


    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## generateOneTimeKeys(): failure - invalid Account ptr");
    }
    else
    {   // keys memory allocation
        randomLength = olm_account_generate_one_time_keys_random_length(accountPtr, (size_t)aNumberOfKeys);
        LOGD("## generateOneTimeKeys(): randomLength=%ld", randomLength);

        if(!setRandomInBuffer(&randomBufferPtr, randomLength))
        {
            LOGE("## generateOneTimeKeys(): failure - random buffer init");
        }
        else
        {
            LOGD("## generateOneTimeKeys(): accountPtr =%p aNumberOfKeys=%d",accountPtr, aNumberOfKeys);

            // retrieve key pairs in keysBytesPtr
            result = olm_account_generate_one_time_keys(accountPtr, (size_t)aNumberOfKeys, (void*)randomBufferPtr, randomLength);
            if(result == olm_error()) {
                const char *errorMsgPtr = olm_account_last_error(accountPtr);
                LOGE("## generateOneTimeKeys(): failure - error generating one time keys Msg=%s",errorMsgPtr);
            }
            else
            {
                retCode = ERROR_CODE_OK;
                LOGD("## generateOneTimeKeys(): success - result=%ld", result);
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

    LOGD("## oneTimeKeys(): accountPtr =%p",accountPtr);

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## oneTimeKeys(): failure - invalid Account ptr");
    }
    else
    {   // keys memory allocation
        keysLength = olm_account_one_time_keys_length(accountPtr);
        if(NULL == (keysBytesPtr=(uint8_t *)malloc(keysLength*sizeof(uint8_t))))
        {
            LOGE("## oneTimeKeys(): failure - one time keys array OOM");
        }
        else
        {   // retrieve key pairs in keysBytesPtr
            keysResult = olm_account_one_time_keys(accountPtr, keysBytesPtr, keysLength);
            if(keysResult == olm_error()) {
                const char *errorMsgPtr = olm_account_last_error(accountPtr);
                LOGE("## oneTimeKeys(): failure - error getting one time keys Msg=%s",errorMsgPtr);
            }
            else
            {   // allocate the byte array to be returned to java
                if(NULL == (byteArrayRetValue=env->NewByteArray(keysLength)))
                {
                    LOGE("## oneTimeKeys(): failure - return byte array OOM");
                }
                else
                {
                    env->SetByteArrayRegion(byteArrayRetValue, 0/*offset*/, keysLength, (const jbyte*)keysBytesPtr);
                    LOGD("## oneTimeKeys(): success");
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
            const char *errorMsgPtr = olm_account_last_error(accountPtr);
            LOGW("## removeOneTimeKeysForSessionJni(): failure - removing one time keys Msg=%s",errorMsgPtr);

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
            const char *errorMsgPtr = olm_account_last_error(accountPtr);
            LOGW("## markOneTimeKeysAsPublishedJni(): failure - Msg=%s",errorMsgPtr);
            retCode = ERROR_CODE_KO;
        }
        else
        {
            LOGD("## markOneTimeKeysAsPublishedJni(): success - retCode=%ld",result);
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
            if(NULL == (signedMsgPtr = (void*)malloc(signatureLength*sizeof(uint8_t))))
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
                    const char *errorMsgPtr = olm_account_last_error(accountPtr);
                    LOGE("## signMessageJni(): failure - error signing message Msg=%s",errorMsgPtr);
                }
                else
                {
                    // TODO check if signedMsgPtr needs to be null ended: signedMsgPtr[resultSign]='\0'
                    // convert to jstring
                    signedMsgRetValue = env->NewStringUTF((const char*)signedMsgPtr); // UTF8
                    LOGD("## signMessageJni(): success - retCode=%ld",resultSign);
                }

                free(signedMsgPtr);
            }

            // release messageToSign
            env->ReleaseStringUTFChars(aMessage, messageToSign);
        }
    }

    return signedMsgRetValue;
}


JNIEXPORT jstring OLM_MANAGER_FUNC_DEF(getOlmLibVersion)(JNIEnv* env, jobject thiz)
{
  uint8_t majorVer=0, minorVer=0, patchVer=0;
  jstring returnValueStr=0;
  char buff[150];

  olm_get_library_version(&majorVer, &minorVer, &patchVer);
  LOGD("## getOlmLibVersion(): Major=%d Minor=%d Patch=%d", majorVer, minorVer, patchVer);

  snprintf(buff, sizeof(buff), " V%d.%d.%d", majorVer, minorVer, patchVer);
  returnValueStr = env->NewStringUTF((const char*)buff);

  return returnValueStr;
}


