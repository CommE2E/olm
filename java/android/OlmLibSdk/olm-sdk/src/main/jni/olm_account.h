#ifndef _OMLACCOUNT_H
#define _OMLACCOUNT_H

#include "olm_jni.h"

#define OLM_ACCOUNT_FUNC_DEF(func_name) FUNC_DEF(OlmAccount,func_name)
#define OLM_MANAGER_FUNC_DEF(func_name) FUNC_DEF(OlmManager,func_name)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring OLM_MANAGER_FUNC_DEF(getOlmLibVersion)(JNIEnv *env, jobject thiz);

// account creation/destruction
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(releaseAccountJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(initNewAccountJni)(JNIEnv *env, jobject thiz);

// identity keys
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(identityKeysJni)(JNIEnv *env, jobject thiz);

// one time keys
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(oneTimeKeysJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(maxOneTimeKeys)(JNIEnv *env, jobject thiz);
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(generateOneTimeKeys)(JNIEnv *env, jobject thiz, jint aNumberOfKeys);
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(removeOneTimeKeysForSession)(JNIEnv *env, jobject thiz, jlong aNativeOlmSessionId);
JNIEXPORT jint OLM_ACCOUNT_FUNC_DEF(markOneTimeKeysAsPublished)(JNIEnv *env, jobject thiz);

// signing
JNIEXPORT jstring OLM_ACCOUNT_FUNC_DEF(signMessage)(JNIEnv *env, jobject thiz, jstring aMessage);

#ifdef __cplusplus
}
#endif

#endif
