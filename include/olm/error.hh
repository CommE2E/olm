/* Copyright 2015 OpenMarket Ltd
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
#ifndef ERROR_HH_
#define ERROR_HH_

namespace olm {

enum struct ErrorCode {
    SUCCESS = 0, /*!< There wasn't an error */
    NOT_ENOUGH_RANDOM = 1,  /*!< Not enough entropy was supplied */
    OUTPUT_BUFFER_TOO_SMALL = 2, /*!< Supplied output buffer is too small */
    BAD_MESSAGE_VERSION = 3,  /*!< The message version is unsupported */
    BAD_MESSAGE_FORMAT = 4, /*!< The message couldn't be decoded */
    BAD_MESSAGE_MAC = 5, /*!< The message couldn't be decrypted */
    BAD_MESSAGE_KEY_ID = 6, /*!< The message references an unknown key id */
    INVALID_BASE64 = 7, /*!< The input base64 was invalid */
    BAD_ACCOUNT_KEY = 8, /*!< The supplied account key is invalid */
};

} // namespace olm

#endif /* ERROR_HH_ */
