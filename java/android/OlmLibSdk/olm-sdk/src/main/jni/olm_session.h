#ifndef _OMLSESSION_H
#define _OMLSESSION_H

#include "olm_jni.h"

#ifdef __cplusplus
extern "C" {
#endif

// session creation/destruction
JNIEXPORT void JNICALL Java_org_matrix_olm_OlmSession_releaseSessionJni(JNIEnv *env, jobject thiz);
JNIEXPORT jlong JNICALL Java_org_matrix_olm_OlmSession_initNewSessionJni(JNIEnv *env, jobject thiz);

// outbound session
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_initOutboundSessionJni(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aTheirOneTimeKey);

// inbound sessions: establishment based on PRE KEY message
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_initInboundSessionJni(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aOneTimeKeyMsg);
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_initInboundSessionFromIdKeyJni(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg);

// match inbound sessions: based on PRE KEY message
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_matchesInboundSessionJni(JNIEnv *env, jobject thiz, jstring aOneTimeKeyMsg);
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_matchesInboundSessionFromIdKeyJni(JNIEnv *env, jobject thiz, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg);

// encrypt/decrypt
JNIEXPORT jint JNICALL Java_org_matrix_olm_OlmSession_encryptMessageJni(JNIEnv *env, jobject thiz, jstring aClearMsg, jobject aEncryptedMsg);
JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmSession_decryptMessageJni(JNIEnv *env, jobject thiz, jobject aEncryptedMsg);

JNIEXPORT jstring JNICALL Java_org_matrix_olm_OlmSession_getSessionIdentifierJni(JNIEnv *env, jobject thiz);

#ifdef __cplusplus
}
#endif

#endif
