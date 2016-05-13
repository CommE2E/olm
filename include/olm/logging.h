/* Copyright 2016 OpenMarket Ltd
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

#ifndef OLM_LOGGING_H_
#define OLM_LOGGING_H_

#include "olm/olm.hh"

#ifdef __cplusplus
extern "C" {
#endif

#define OLM_LOG_FATAL   1
#define OLM_LOG_ERROR   2
#define OLM_LOG_WARNING 3
#define OLM_LOG_INFO    4
#define OLM_LOG_DEBUG   5
#define OLM_LOG_TRACE   6

/* returns non-zero if logging is enabled for this level */
int olm_log_enabled_for(unsigned int level, const char *category);

__attribute__((__format__ (__printf__, 3, 4)))
void olm_logf(unsigned int level, const char *category,
          const char *format, ...);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OLM_LOGGING_H_ */
