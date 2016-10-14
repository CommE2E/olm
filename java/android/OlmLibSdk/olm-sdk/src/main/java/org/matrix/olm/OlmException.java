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

package org.matrix.olm;

public class OlmException extends Exception {
    // exception codes
    public static final int EXCEPTION_CODE_INIT_NEW_SESSION_FAILURE = 0;
    public static final int EXCEPTION_CODE_INIT_OUTBOUND_GROUP_SESSION = 1;
    public static final int EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION = 2;

    private final int mCode;

    public OlmException(int aExceptionCode) {
        super();
        mCode = aExceptionCode;
    }

    public int getExceptionCode() {
        return mCode;
    }
}
