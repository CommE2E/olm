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

#ifndef _OMLACCOUNT_H
#define _OMLACCOUNT_H

#include "olm_jni.h"
#include "olm/olm.h"

#define OLM_ACCOUNT_FUNC_DEF(func_name) FUNC_DEF(OlmAccount,func_name)
#define OLM_MANAGER_FUNC_DEF(func_name) FUNC_DEF(OlmManager,func_name)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring OLM_MANAGER_FUNC_DEF(getOlmLibVersion)(JNIEnv *env, jobject thiz);

// account creation/destruction
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(releaseAccountJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(initNewAccountJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(createNewAccountJni)(JNIEnv *env, jobject thiz);

// identity keys
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(identityKeysJni)(JNIEnv *env, jobject thiz);

// one time keys
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(oneTimeKeysJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(maxOneTimeKeys)(JNIEnv *env, jobject thiz);
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(generateOneTimeKeys)(JNIEnv *env, jobject thiz, jint aNumberOfKeys);
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(removeOneTimeKeysForSessionJni)(JNIEnv *env, jobject thiz, jlong aNativeOlmSessionId);
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(markOneTimeKeysAsPublishedJni)(JNIEnv *env, jobject thiz);

// signing
JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(signMessageJni)(JNIEnv *env, jobject thiz, jstring aMessage);

// serialization
JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(serializeDataWithKeyJni)(JNIEnv *env, jobject thiz, jstring aKey, jobject aErrorMsg);
JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(initWithSerializedDataJni)(JNIEnv *env, jobject thiz, jstring aSerializedData, jstring aKey);

#ifdef __cplusplus
}
#endif

#endif
