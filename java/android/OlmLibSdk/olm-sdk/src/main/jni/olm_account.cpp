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
* @return valid memory alocation, NULL otherwise
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
JNIEXPORT void JNICALL Java_org_matrix_olm_OlmAccount_releaseAccountJni(JNIEnv *env, jobject thiz)
{
  OlmAccount* accountPtr = NULL;

  LOGD("## releaseAccountJni(): accountPtr=%p",accountPtr);

  if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
  {
      LOGE("## releaseAccountJni(): failure - invalid Account ptr=NULL");
  }
  else
  { // even if free(NULL) does not crash, a test is performed for debug purpose
    LOGD("## releaseAccountJni(): IN");
    free(accountPtr);
    LOGD("## releaseAccountJni(): OUT");
  }
}

/**
* Initialize a new account and return it to JAVA side.<br>
* Since a C prt is returned as a jlong, special care will be taken
* to make the cast (OlmAccount* => jlong) platform independant.
* @return the initialized OlmAccount* instance if init succeed, NULL otherwise
**/
JNIEXPORT jlong JNICALL Java_org_matrix_olm_OlmAccount_initNewAccountJni(JNIEnv *env, jobject thiz)
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
        if(false == setRandomInBuffer(&randomBuffPtr, randomSize))
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
JNIEXPORT jbyteArray JNICALL Java_org_matrix_olm_OlmAccount_identityKeysJni(JNIEnv *env, jobject thiz)
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
        if(NULL == (identityKeysBytesPtr=(uint8_t *)malloc(identityKeysLength*sizeof(std::uint8_t))))
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
JNIEXPORT jlong JNICALL Java_org_matrix_olm_OlmAccount_maxOneTimeKeys(JNIEnv *env, jobject thiz)
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
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmAccount_generateOneTimeKeys(JNIEnv *env, jobject thiz, jint aNumberOfKeys)
{
    OlmAccount *accountPtr = NULL;
    uint8_t *randomBufferPtr = NULL;
    jint retCode = ERROR_CODE_KO;
    size_t randomLength;
    size_t result;

    LOGD("## generateOneTimeKeys(): accountPtr =%p aNumberOfKeys=%d",accountPtr, aNumberOfKeys);

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## generateOneTimeKeys(): failure - invalid Account ptr");
    }
    else
    {   // keys memory allocation
        randomLength = olm_account_generate_one_time_keys_random_length(accountPtr, aNumberOfKeys);
        LOGD("## generateOneTimeKeys(): randomLength=%ld", randomLength);

        if(false == setRandomInBuffer(&randomBufferPtr, randomLength))
        {
            LOGE("## generateOneTimeKeys(): failure - random buffer init");
        }
        else
        {   // retrieve key pairs in keysBytesPtr
            result = olm_account_generate_one_time_keys(accountPtr, aNumberOfKeys, (void*)randomBufferPtr, randomLength);
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
 * Get "one time keys".
 * Return the public parts of the unpublished "one time keys" for the account
 * @return a valid byte array if operation succeed, null otherwise
**/
JNIEXPORT jbyteArray JNICALL Java_org_matrix_olm_OlmAccount_oneTimeKeysJni(JNIEnv *env, jobject thiz)
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
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmAccount_removeOneTimeKeysForSession(JNIEnv *env, jobject thiz, jlong aNativeOlmSessionId)
{
    jint retCode = ERROR_CODE_KO;
    OlmAccount* accountPtr = NULL;
    OlmSession* sessionPtr = (OlmSession*)aNativeOlmSessionId;
    size_t result;

    if(NULL == sessionPtr)
    {
        LOGE("## removeOneTimeKeysForSession(): failure - invalid session ptr");
    }
    else if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## removeOneTimeKeysForSession(): failure - invalid account ptr");
    }
    else
    {
        result = olm_remove_one_time_keys(accountPtr, sessionPtr);
        if(result == olm_error())
        {   // the account doesn't have any matching "one time keys"..
            const char *errorMsgPtr = olm_account_last_error(accountPtr);
            LOGW("## removeOneTimeKeysForSession(): failure - removing one time keys Msg=%s",errorMsgPtr);

            retCode = ERROR_CODE_NO_MATCHING_ONE_TIME_KEYS;
        }
        else
        {
            retCode = ERROR_CODE_OK;
            LOGD("## removeOneTimeKeysForSession(): success");
        }
    }

    return retCode;
}

/**
 * Mark the current set of "one time keys" as being published.
 * @return ERROR_CODE_OK if operation succeed, ERROR_CODE_KO otherwise
**/
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmAccount_markOneTimeKeysAsPublished(JNIEnv *env, jobject thiz)
{
    jint retCode = ERROR_CODE_OK;
    OlmAccount* accountPtr = NULL;
    size_t result;

    if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## markOneTimeKeysPublished(): failure - invalid account ptr");
        retCode = ERROR_CODE_KO;
    }
    else
    {
        result = olm_account_mark_keys_as_published(accountPtr);
        if(result == olm_error())
        {
            const char *errorMsgPtr = olm_account_last_error(accountPtr);
            LOGW("## markOneTimeKeysPublished(): failure - Msg=%s",errorMsgPtr);
            retCode = ERROR_CODE_KO;
        }
        else
        {
            LOGD("## markOneTimeKeysPublished(): success - retCode=%ld",result);
        }
    }

    return retCode;
}

/**
 * Sign a message with the ed25519 key (fingerprint) for this account.
 * @param aMessage message to sign
 * @return the corresponding signed message, null otherwise
**/
JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmAccount_signMessage(JNIEnv *env, jobject thiz, jstring aMessage)
{
    OlmAccount* accountPtr = NULL;
    size_t signatureLength;
    void* signaturePtr;
    size_t resultSign;
    jstring signedMsgRetValue = NULL;

    if(NULL == aMessage)
    {
        LOGE("## signMessage(): failure - invalid aMessage param");
    }
    else if(NULL == (accountPtr = (OlmAccount*)getAccountInstanceId(env,thiz)))
    {
        LOGE("## signMessage(): failure - invalid account ptr");
    }
    else
    {
        // convert message from JAVA to C string
        const char* messageToSign = env->GetStringUTFChars(aMessage, 0);
        if(NULL == messageToSign)
        {
            LOGE("## signMessage(): failure - message JNI allocation OOM");
        }
        else
        {
            int messageLength = env->GetStringUTFLength(aMessage);

            // signature memory allocation
            signatureLength = olm_account_signature_length(accountPtr);
            if(NULL == (signaturePtr=(void *)malloc(signatureLength*sizeof(void*))))
            {
                LOGE("## signMessage(): failure - signature allocation OOM");
            }
            else
            {   // sign message
                resultSign = olm_account_sign(accountPtr, (void*)messageToSign, messageLength, signaturePtr, signatureLength);
                if(resultSign == olm_error())
                {
                    const char *errorMsgPtr = olm_account_last_error(accountPtr);
                    LOGE("## signMessage(): failure - error signing message Msg=%s",errorMsgPtr);
                }
                else
                {   // convert to jstring
                    // TODO check how UTF conversion can impact the content?
                    // why not consider return jbyteArray? and convert in JAVA side..
                    signedMsgRetValue = env->NewStringUTF((const char*)signaturePtr); // UTF8
                    LOGD("## signMessage(): success - retCode=%ld",resultSign);
                }

                free(signaturePtr);
            }

            // release messageToSign
            env->ReleaseStringUTFChars(aMessage, messageToSign);
        }
    }

    return signedMsgRetValue;
}


JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmManager_getOlmLibVersion(JNIEnv* env, jobject thiz)
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


/**
* Read the account instance ID of the calling object.
* @return the instance ID if read succeed, -1 otherwise.
**/
jlong getAccountInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId=-1;
  jfieldID instanceIdField;
  jclass loaderClass;

  if(NULL!=aJniEnv)
  {
    if(0 != (loaderClass=aJniEnv->GetObjectClass(aJavaObject)))
    {
      if(0 != (instanceIdField=aJniEnv->GetFieldID(loaderClass, "mNativeOlmAccountId", "J")))
      {
        instanceId = aJniEnv->GetLongField(aJavaObject, instanceIdField);
        aJniEnv->DeleteLocalRef(loaderClass);
        LOGD("## getAccountInstanceId(): read from java instanceId=%lld",instanceId);
      }
      else
      {
        LOGD("## getAccountInstanceId() ERROR! GetFieldID=null");
      }
    }
    else
    {
      LOGD("## getAccountInstanceId() ERROR! GetObjectClass=null");
    }
  }
  else
  {
    LOGD("## getAccountInstanceId() ERROR! aJniEnv=NULL");
  }
  LOGD("## getAccountInstanceId() success - instanceId=%lld",instanceId);
  return instanceId;
}
