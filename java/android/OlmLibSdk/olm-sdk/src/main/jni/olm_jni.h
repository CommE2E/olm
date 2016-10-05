#ifndef _OMLJNI_H
#define _OMLJNI_H

#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <map>
#include <jni.h>
#include <android/log.h>

#include "olm/olm.h"

#define TAG "OlmJniNative"

/* logging macros */
#define ENABLE_LOGS

#ifdef ENABLE_LOGS
    #define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
    #define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
    #define LOGV(...)
    #define LOGD(...)
    #define LOGW(...)
    #define LOGE(...)
#endif

// Error codes definition
static const int ERROR_CODE_OK = 0;
static const int ERROR_CODE_NO_MATCHING_ONE_TIME_KEYS = ERROR_CODE_OK+1;
static const int ERROR_CODE_KO = -1;

// constants
static const int ACCOUNT_CREATION_RANDOM_MODULO = 500;


typedef struct _AccountContext
{
  OlmAccount* mAccountPtr;
  _AccountContext(): mAccountPtr(NULL){}
} AccountContext;

#endif
