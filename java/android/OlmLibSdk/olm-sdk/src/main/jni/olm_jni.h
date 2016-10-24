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

#ifndef _OMLJNI_H
#define _OMLJNI_H

#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <jni.h>
#include <android/log.h>


#define TAG "OlmJniNative"

/* logging macros */
//#define ENABLE_LOGS

#ifdef NDK_DEBUG
    #warning "NDK_DEBUG is defined"
#else
    #warning "NDK_DEBUG not defined"
#endif

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#ifdef NDK_DEBUG
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
    #define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#else
    #define LOGD(...)
    #define LOGW(...)
#endif

#define FUNC_DEF(class_name,func_name) JNICALL Java_org_matrix_olm_##class_name##_##func_name

namespace AndroidOlmSdk
{
    // Error codes definition
    static const int ERROR_CODE_OK = 0;
    static const int ERROR_CODE_NO_MATCHING_ONE_TIME_KEYS = ERROR_CODE_OK+1;
    static const int ERROR_CODE_KO = -1;

    // constants
    static const int ACCOUNT_CREATION_RANDOM_MODULO = 256;
}


// function pointer templates
template<typename T> using olmPickleLengthFuncPtr = size_t (*)(T);
template<typename T> using olmPickleFuncPtr = size_t (*)(T, void const *, size_t, void *, size_t);
template<typename T> using olmLastErrorFuncPtr = const char* (*)(T);

template <typename T>
jstring serializeDataWithKey(JNIEnv *env, jobject thiz,
                                jstring aKey,
                                jobject aErrorMsg,
                                olmPickleLengthFuncPtr<T> aGetLengthFunc,
                                olmPickleFuncPtr<T> aGetPickleFunc,
                                olmLastErrorFuncPtr<T> aGetLastErrorFunc);

#ifdef __cplusplus
extern "C" {
#endif

// internal helper functions
bool setRandomInBuffer(uint8_t **aBuffer2Ptr, size_t aRandomSize);
jlong getSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);
jlong getAccountInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);
jlong getInboundGroupSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);
jlong getOutboundGroupSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);
jlong getUtilityInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);

#ifdef __cplusplus
}
#endif


#endif
