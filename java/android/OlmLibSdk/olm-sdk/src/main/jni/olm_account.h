#ifndef _OMLACCOUNT_H
#define _OMLACCOUNT_H

#include "olm_jni.h"

#define OLM_ACCOUNT_FUNC_DEF(func_name) FUNC_DEF(OlmAccount,func_name)
#define OLM_MANAGER_FUNC_DEF(func_name) FUNC_DEF(OlmManager,func_name)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmManager_getOlmLibVersion(JNIEnv *env, jobject thiz);

// account creation/destruction
JNIEXPORT void JNICALL Java_org_matrix_olm_OlmAccount_releaseAccountJni(JNIEnv *env, jobject thiz);
JNIEXPORT jlong JNICALL Java_org_matrix_olm_OlmAccount_initNewAccountJni(JNIEnv *env, jobject thiz);

// identity keys
JNIEXPORT jbyteArray JNICALL Java_org_matrix_olm_OlmAccount_identityKeysJni(JNIEnv *env, jobject thiz);

// one time keys
JNIEXPORT jbyteArray JNICALL Java_org_matrix_olm_OlmAccount_oneTimeKeysJni(JNIEnv *env, jobject thiz);
JNIEXPORT jlong JNICALL Java_org_matrix_olm_OlmAccount_maxOneTimeKeys(JNIEnv *env, jobject thiz);
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmAccount_generateOneTimeKeys(JNIEnv *env, jobject thiz, jint aNumberOfKeys);
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmAccount_removeOneTimeKeysForSession(JNIEnv *env, jobject thiz, jlong aNativeOlmSessionId);
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmAccount_markOneTimeKeysAsPublished(JNIEnv *env, jobject thiz);

// signing
JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmAccount_signMessage(JNIEnv *env, jobject thiz, jstring aMessage);

#ifdef __cplusplus
}
#endif

#endif
