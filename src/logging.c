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

#include "olm/logging.h"

#include <stdarg.h>
#include <stdio.h>

static unsigned int log_level = 1;

void olm_set_log_level(unsigned int level) {
    log_level = level;
}

int olm_log_enabled_for(unsigned int level, const char *category)
{
    return level <= log_level;
}

void olm_logf(unsigned int level, const char *category,
          const char *format, ...) {
    if (level > log_level) {
        return;
    }

    fputs(category, stdout);
    fputs(": ", stdout);

    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);

    putchar('\n');
}
