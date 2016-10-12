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

#include "olm_jni.h"
#include "olm_utility.h"

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
        LOGD("## setRandomInBuffer(): failure - aBuffer=NULL");
    }
    else if(0 == aRandomSize)
    {
        LOGD("## setRandomInBuffer(): failure - random size=0");
    }
    else if(NULL == (*aBuffer2Ptr = (uint8_t*)malloc(aRandomSize*sizeof(uint8_t))))
    {
        LOGD("## setRandomInBuffer(): failure - alloc mem OOM");
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
* Read the account instance ID of the calling object.
* @param aJniEnv pointer pointing on the JNI function table
* @param aJavaObject reference to the object on which the method is invoked
* @return the instance ID if operation succeed, -1 if instance ID was not found.
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

/**
* Read the account instance ID of the calling object (aJavaObject).<br>
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
        instanceId = aJniEnv->GetLongField(aJavaObject, instanceIdField);
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

  //LOGD("## getSessionInstanceId() success - instanceId=%lld",instanceId);
  return instanceId;
}
