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

#define FUNC_DEF(class_name,func_name) JNICALL Java_org_matrix_olm_##class_name##_##func_name

// Error codes definition
static const int ERROR_CODE_OK = 0;
static const int ERROR_CODE_NO_MATCHING_ONE_TIME_KEYS = ERROR_CODE_OK+1;
static const int ERROR_CODE_KO = -1;

// constants
static const int ACCOUNT_CREATION_RANDOM_MODULO = 256;


typedef struct _AccountContext
{
  OlmAccount* mAccountPtr;
  _AccountContext(): mAccountPtr(NULL){}
} AccountContext;

#endif
