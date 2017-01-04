/*
 * Copyright 2016 OpenMarket Ltd
 * Copyright 2016 Vector Creations Ltd
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

import java.io.IOException;

/**
 * Exception class to identify specific Olm SDK exceptions.
 */
public class OlmException extends IOException {
    // exception codes

    public static final int EXCEPTION_CODE_INIT_ACCOUNT_CREATION = 10;

    public static final int EXCEPTION_CODE_ACCOUNT_SERIALIZATION = 20;
    public static final int EXCEPTION_CODE_ACCOUNT_DESERIALIZATION = 21;
    public static final int EXCEPTION_CODE_ACCOUNT_IDENTITY_KEYS = 22;
    public static final int EXCEPTION_CODE_ACCOUNT_GENERATE_ONE_TIME_KEYS = 23;
    public static final int EXCEPTION_CODE_ACCOUNT_ONE_TIME_KEYS = 24;
    public static final int EXCEPTION_CODE_ACCOUNT_REMOVE_ONE_TIME_KEYS = 25;
    public static final int EXCEPTION_CODE_ACCOUNT_MARK_ONE_KEYS_AS_PUBLISHED = 26;
    public static final int EXCEPTION_CODE_ACCOUNT_SIGN_MESSAGE = 27;

    public static final int EXCEPTION_CODE_CREATE_INBOUND_GROUP_SESSION = 30;
    public static final int EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION = 31;
    public static final int EXCEPTION_CODE_INBOUND_GROUP_SESSION_IDENTIFIER = 32;
    public static final int EXCEPTION_CODE_INBOUND_GROUP_SESSION_DECRYPT_SESSION = 33;

    public static final int EXCEPTION_CODE_CREATE_OUTBOUND_GROUP_SESSION = 40;
    public static final int EXCEPTION_CODE_INIT_OUTBOUND_GROUP_SESSION = 41;
    public static final int EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_IDENTIFIER = 42;
    public static final int EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_KEY = 43;
    public static final int EXCEPTION_CODE_OUTBOUND_GROUP_ENCRYPT_MESSAGE = 44;

    public static final int EXCEPTION_CODE_INIT_SESSION_CREATION = 50;
    public static final int EXCEPTION_CODE_SESSION_INIT_OUTBOUND_SESSION = 51;
    public static final int EXCEPTION_CODE_SESSION_INIT_INBOUND_SESSION = 52;
    public static final int EXCEPTION_CODE_SESSION_INIT_INBOUND_SESSION_FROM = 53;
    public static final int EXCEPTION_CODE_SESSION_ENCRYPT_MESSAGE = 54;
    public static final int EXCEPTION_CODE_SESSION_DECRYPT_MESSAGE = 55;
    public static final int EXCEPTION_CODE_SESSION_SESSION_IDENTIFIER = 56;

    // exception human readable messages
    public static final String EXCEPTION_MSG_NEW_OUTBOUND_GROUP_SESSION = "createNewSession() failed";
    public static final String EXCEPTION_MSG_NEW_INBOUND_GROUP_SESSION = "createNewSession() failed";
    public static final String EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION = "invalid de-serialized parameters";
    public static final String EXCEPTION_MSG_INIT_ACCOUNT_CREATION = "initNewAccount() failed";
    public static final String EXCEPTION_MSG_INIT_SESSION_CREATION = "initNewSession() failed";

    /** exception code to be taken from: {@link #EXCEPTION_CODE_CREATE_OUTBOUND_GROUP_SESSION}, {@link #EXCEPTION_CODE_CREATE_INBOUND_GROUP_SESSION},
     * {@link #EXCEPTION_CODE_INIT_OUTBOUND_GROUP_SESSION}, {@link #EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION}..**/
    private final int mCode;

    /** Human readable message description **/
    private final String mMessage;

    public OlmException(int aExceptionCode, String aExceptionMessage) {
        super();
        mCode = aExceptionCode;
        mMessage = aExceptionMessage;
    }

    public int getExceptionCode() {
        return mCode;
    }

    @Override
    public String getMessage() {
        return mMessage;
    }
}
