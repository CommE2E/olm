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

#ifndef OLM_LOGGING_HH_
#define OLM_LOGGING_HH_

namespace olm {

const unsigned int LOG_FATAL   = 1;
const unsigned int LOG_ERROR   = 2;
const unsigned int LOG_WARNING = 3;
const unsigned int LOG_INFO    = 4;
const unsigned int LOG_DEBUG   = 5;

void set_log_level(unsigned int log_level);

__attribute__((__format__ (__printf__, 3, 4)))
void logf(unsigned int level, const char *category,
          const char *format, ...);

} // namespace olm

#endif /* OLM_LOGGING_HH_ */
