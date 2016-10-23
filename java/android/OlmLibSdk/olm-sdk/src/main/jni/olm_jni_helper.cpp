/**
 * Created by pedrocon on 06/10/2016.
 */
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

#include "olm_jni_helper.h"
#include "olm/olm.h"

using namespace AndroidOlmSdk;

/**
* Init a buffer with a given number of random values.
* @param aBuffer2Ptr the buffer to be initialized
* @param aRandomSize the number of random values to apply
* @return true if operation succeed, false otherwise
**/
bool setRandomInBuffer(uint8_t **aBuffer2Ptr, size_t aRandomSize)
{
    bool retCode = false;
    if(NULL == aBuffer2Ptr)
    {
        LOGE("## setRandomInBuffer(): failure - aBuffer=NULL");
    }
    else if(0 == aRandomSize)
    {
        LOGE("## setRandomInBuffer(): failure - random size=0");
    }
    else if(NULL == (*aBuffer2Ptr = (uint8_t*)malloc(aRandomSize*sizeof(uint8_t))))
    {
        LOGE("## setRandomInBuffer(): failure - alloc mem OOM");
    }
    else
    {
        LOGD("## setRandomInBuffer(): randomSize=%ld",aRandomSize);

        srand(time(NULL)); // init seed
        for(size_t i=0;i<aRandomSize;i++)
        {
            (*aBuffer2Ptr)[i] = (uint8_t)(rand()%ACCOUNT_CREATION_RANDOM_MODULO);

            // debug purpose
            //LOGD("## setRandomInBuffer(): randomBuffPtr[%ld]=%d",i, (*aBuffer2Ptr)[i]);
        }

        retCode = true;
    }
    return retCode;
}


/**
* Read the instance ID of the calling object.
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @param aCallingClass java calling clas name
* @return the instance ID if operation succeed, -1 if instance ID was not found.
**/
jlong getInstanceId(JNIEnv* aJniEnv, jobject aJavaObject, const char *aCallingClass)
{
  jlong instanceId = 0;
  jfieldID instanceIdField;
  jclass loaderClass;
  jclass requiredClass = 0;

  if(NULL!=aJniEnv)
  {
    requiredClass = aJniEnv->FindClass(aCallingClass);

    if((0 != requiredClass) && (JNI_TRUE != aJniEnv->IsInstanceOf(aJavaObject, requiredClass)))
    {
        LOGE("## getAccountInstanceId() failure - invalid instance of");
    }
    else if(0 != (loaderClass=aJniEnv->GetObjectClass(aJavaObject)))
    {
      if(0 != (instanceIdField=aJniEnv->GetFieldID(loaderClass, "mNativeId", "J")))
      {
        instanceId = aJniEnv->GetLongField(aJavaObject, instanceIdField);
        aJniEnv->DeleteLocalRef(loaderClass);
        LOGD("## getInstanceId(): read from java instanceId=%lld",instanceId);
      }
      else
      {
        LOGE("## getInstanceId() ERROR! GetFieldID=null");
      }
    }
    else
    {
      LOGE("## getInstanceId() ERROR! GetObjectClass=null");
    }
  }
  else
  {
    LOGE("## getInstanceId() ERROR! aJniEnv=NULL");
  }
  LOGD("## getInstanceId() success - instanceId=%p (jlong)(intptr_t)instanceId=%lld",(void*)instanceId, (jlong)(intptr_t)instanceId);
  return instanceId;
}

/**
* Read the account instance ID of the calling object.
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if operation succeed, -1 if instance ID was not found.
**/
jlong getAccountInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId = getInstanceId(aJniEnv, aJavaObject, CLASS_OLM_ACCOUNT);
  return instanceId;
}



/**
* Read the session instance ID of the calling object (aJavaObject).<br>
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if read succeed, -1 otherwise.
**/
jlong getSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId = getInstanceId(aJniEnv, aJavaObject, CLASS_OLM_SESSION);
  return instanceId;
}

/**
* Read the inbound group session instance ID of the calling object (aJavaObject).<br>
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if read succeed, -1 otherwise.
**/
jlong getInboundGroupSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId = getInstanceId(aJniEnv, aJavaObject, CLASS_OLM_INBOUND_GROUP_SESSION);
  return instanceId;
}


/**
* Read the outbound group session instance ID of the calling object (aJavaObject).<br>
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if read succeed, -1 otherwise.
**/
jlong getOutboundGroupSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId = getInstanceId(aJniEnv, aJavaObject, CLASS_OLM_OUTBOUND_GROUP_SESSION);
  return instanceId;
}

/**
* Read the utility instance ID of the calling object (aJavaObject).<br>
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if read succeed, -1 otherwise.
**/
jlong getUtilityInstanceId(JNIEnv* aJniEnv, jobject aJavaObject)
{
  jlong instanceId = getInstanceId(aJniEnv, aJavaObject, CLASS_OLM_UTILITY);
  return instanceId;
}


template <typename T>
jstring serializeDataWithKey(JNIEnv *env, jobject thiz,
                                jstring aKey,
                                jobject aErrorMsg,
                                olmPickleLengthFuncPtr<T> aGetLengthFunc,
                                olmPickleFuncPtr<T> aGetPickleFunc,
                                olmLastErrorFuncPtr<T> aGetLastErrorFunc)
{
    jstring pickledDataRetValue = 0;
    jclass errorMsgJClass = 0;
    jmethodID errorMsgMethodId = 0;
    jstring errorJstring = 0;
    const char *keyPtr = NULL;
    void *pickledPtr = NULL;
    T accountPtr = NULL;

    LOGD("## serializeDataWithKeyJni(): IN");

    if(NULL == (accountPtr = (T)getAccountInstanceId(env,thiz)))
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
        size_t pickledLength = aGetLengthFunc(accountPtr);
        size_t keyLength = (size_t)env->GetStringUTFLength(aKey);
        LOGD(" ## serializeDataWithKeyJni(): pickledLength=%lu keyLength=%lu",pickledLength, keyLength);
        LOGD(" ## serializeDataWithKeyJni(): key=%s",(char const *)keyPtr);

        if(NULL == (pickledPtr = (void*)malloc((pickledLength+1)*sizeof(uint8_t))))
        {
            LOGE(" ## serializeDataWithKeyJni(): failure - pickledPtr buffer OOM");
        }
        else
        {
            size_t result = aGetPickleFunc(accountPtr,
                                           (void const *)keyPtr,
                                           keyLength,
                                           (void*)pickledPtr,
                                           pickledLength);
            if(result == olm_error())
            {
                const char *errorMsgPtr = aGetLastErrorFunc(accountPtr);
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