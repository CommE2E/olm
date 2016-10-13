#ifndef _OMLSESSION_H
#define _OMLSESSION_H

#include "olm_jni.h"

#define OLM_SESSION_FUNC_DEF(func_name) FUNC_DEF(OlmSession,func_name)

#ifdef __cplusplus
extern "C" {
#endif

// session creation/destruction
JNIEXPORT void OLM_SESSION_FUNC_DEF(releaseSessionJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jlong OLM_SESSION_FUNC_DEF(initNewSessionJni)(JNIEnv *env, jobject thiz);

// outbound session
JNIEXPORT jint OLM_SESSION_FUNC_DEF(initOutboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aTheirOneTimeKey);

// inbound sessions: establishment based on PRE KEY message
JNIEXPORT jint OLM_SESSION_FUNC_DEF(initInboundSessionJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aOneTimeKeyMsg);
JNIEXPORT jint OLM_SESSION_FUNC_DEF(initInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jlong aOlmAccountId, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg);

// match inbound sessions: based on PRE KEY message
JNIEXPORT jint OLM_SESSION_FUNC_DEF(matchesInboundSessionJni)(JNIEnv *env, jobject thiz, jstring aOneTimeKeyMsg);
JNIEXPORT jint OLM_SESSION_FUNC_DEF(matchesInboundSessionFromIdKeyJni)(JNIEnv *env, jobject thiz, jstring aTheirIdentityKey, jstring aOneTimeKeyMsg);

// encrypt/decrypt
JNIEXPORT jint OLM_SESSION_FUNC_DEF(encryptMessageJni)(JNIEnv *env, jobject thiz, jstring aClearMsg, jobject aEncryptedMsg);
JNIEXPORT jstring OLM_SESSION_FUNC_DEF(decryptMessage)(JNIEnv *env, jobject thiz, jobject aEncryptedMsg);

JNIEXPORT jstring OLM_SESSION_FUNC_DEF(getSessionIdentifierJni)(JNIEnv *env, jobject thiz);

#ifdef __cplusplus
}
#endif

#endif
