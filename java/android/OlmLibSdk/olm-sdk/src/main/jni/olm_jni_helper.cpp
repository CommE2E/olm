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

#include "olm_jni_helper.h"
#include "olm/olm.h"
#include <sys/time.h>

using namespace AndroidOlmSdk;

/**
* Init a buffer with a given number of random values.
* @param aBuffer2Ptr the buffer to be initialized
* @param aRandomSize the number of random values to apply
* @return true if operation succeed, false otherwise
**/
bool setRandomInBuffer(JNIEnv *env, uint8_t **aBuffer2Ptr, size_t aRandomSize)
{
    bool retCode = false;
    int bufferLen = aRandomSize*sizeof(uint8_t);

    if (!aBuffer2Ptr)
    {
        LOGE("## setRandomInBuffer(): failure - aBuffer=NULL");
    }
    else if(!aRandomSize)
    {
        LOGE("## setRandomInBuffer(): failure - random size=0");
    }
    else if (!(*aBuffer2Ptr = (uint8_t*)malloc(bufferLen)))
    {
        LOGE("## setRandomInBuffer(): failure - alloc mem OOM");
    }
    else
    {
        LOGD("## setRandomInBuffer(): randomSize=%lu",static_cast<long unsigned int>(aRandomSize));

        // use the secureRandom class
        jclass cls = env->FindClass("java/security/SecureRandom");

        if (cls)
        {
            jobject newObj = 0;
            jmethodID constructor = env->GetMethodID(cls, "<init>", "()V");
            jmethodID nextByteMethod = env->GetMethodID(cls, "nextBytes", "([B)V");

            if (constructor)
            {
                newObj = env->NewObject(cls, constructor);
                jbyteArray tempByteArray = env->NewByteArray(bufferLen);

                if (newObj && tempByteArray)
                {
                    env->CallVoidMethod(newObj, nextByteMethod, tempByteArray);
                    jbyte* buffer = env->GetByteArrayElements(tempByteArray, NULL);

                    if (buffer)
                    {
                        memcpy(*aBuffer2Ptr, buffer, bufferLen);
                        retCode = true;

                        // clear tempByteArray to hide sensitive data.
                        memset(buffer, 0, bufferLen);
                        env->SetByteArrayRegion(tempByteArray, 0, bufferLen, buffer);

                        // ensure that the buffer is released
                        env->ReleaseByteArrayElements(tempByteArray, buffer, JNI_ABORT);
                    }
                }

                if (tempByteArray)
                {
                    env->DeleteLocalRef(tempByteArray);
                }

                if (newObj)
                {
                    env->DeleteLocalRef(newObj);
                }
            }
        }

        // debug purpose
        /*for(int i = 0; i < aRandomSize; i++)
        {
            LOGD("## setRandomInBuffer(): randomBuffPtr[%ld]=%d",i, (*aBuffer2Ptr)[i]);
        }*/
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
    if  (aJniEnv)
    {
        jclass requiredClass = aJniEnv->FindClass(aCallingClass);
        jclass loaderClass = 0;

        if (requiredClass && (JNI_TRUE != aJniEnv->IsInstanceOf(aJavaObject, requiredClass)))
        {
            LOGE("## getAccountInstanceId() failure - invalid instance of");
        }
        else if (loaderClass = aJniEnv->GetObjectClass(aJavaObject))
        {
            jfieldID instanceIdField = aJniEnv->GetFieldID(loaderClass, "mNativeId", "J");

            if (instanceIdField)
            {
                instanceId = aJniEnv->GetLongField(aJavaObject, instanceIdField);
                LOGD("## getInstanceId(): read from java instanceId=%lld",instanceId);
            }
            else
            {
                LOGE("## getInstanceId() ERROR! GetFieldID=null");
            }

             aJniEnv->DeleteLocalRef(loaderClass);
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

/**
* Convert a C string into a UTF-8 format string.
* The conversion is performed in JAVA side to workaround the issue in  NewStringUTF().
* The problem is described here: https://github.com/eclipsesource/J2V8/issues/142
*/
jstring javaCStringToUtf8(JNIEnv *env, uint8_t *aCStringMsgPtr, size_t aMsgLength)
{
    jstring convertedRetValue = 0;
    jbyteArray tempByteArray = NULL;

    if (!aCStringMsgPtr || !env)
    {
        LOGE("## javaCStringToUtf8(): failure - invalid parameters (null)");
    }
    else if (!(tempByteArray=env->NewByteArray(aMsgLength)))
    {
        LOGE("## javaCStringToUtf8(): failure - return byte array OOM");
    }
    else
    {
        env->SetByteArrayRegion(tempByteArray, 0, aMsgLength, (const jbyte*)aCStringMsgPtr);

        // UTF-8 conversion from JAVA
        jstring strEncode = (env)->NewStringUTF("UTF-8");
        jclass jClass = env->FindClass("java/lang/String");
        jmethodID cstor = env->GetMethodID(jClass, "<init>", "([BLjava/lang/String;)V");

        if (jClass && strEncode)
        {
            convertedRetValue = (jstring) env->NewObject(jClass, cstor, tempByteArray, strEncode);
            LOGD(" ## javaCStringToUtf8(): succeed");
            env->DeleteLocalRef(tempByteArray);
        }
        else
        {
            LOGE(" ## javaCStringToUtf8(): failure - invalid Java references");
        }
    }

    return convertedRetValue;
}
