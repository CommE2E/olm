#ifndef _OMLUTILITY_H
#define _OMLUTILITY_H


#ifdef __cplusplus
extern "C" {
#endif

bool setRandomInBuffer(uint8_t **aBuffer2Ptr, size_t aRandomSize);
jlong getSessionInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);
jlong getAccountInstanceId(JNIEnv* aJniEnv, jobject aJavaObject);

#ifdef __cplusplus
}
#endif


#endif
