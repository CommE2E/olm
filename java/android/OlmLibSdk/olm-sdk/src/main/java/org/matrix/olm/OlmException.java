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
    public static final int EXCEPTION_CODE_CREATE_OUTBOUND_GROUP_SESSION = 0;
    public static final int EXCEPTION_CODE_CREATE_INBOUND_GROUP_SESSION = 1;
    public static final int EXCEPTION_CODE_INIT_OUTBOUND_GROUP_SESSION = 2;
    public static final int EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION = 3;
    public static final int EXCEPTION_CODE_ACCOUNT_SERIALIZATION = 4;
    public static final int EXCEPTION_CODE_ACCOUNT_DESERIALIZATION = 5;
    public static final int EXCEPTION_CODE_SESSION_SERIALIZATION = 6;
    public static final int EXCEPTION_CODE_SESSION_DESERIALIZATION = 7;
    public static final int EXCEPTION_CODE_INIT_ACCOUNT_CREATION = 8;
    public static final int EXCEPTION_CODE_INIT_SESSION_CREATION = 9;
    public static final int EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_SERIALIZATION = 10;
    public static final int EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_DESERIALIZATION = 11;
    public static final int EXCEPTION_CODE_INBOUND_GROUP_SESSION_SERIALIZATION = 12;
    public static final int EXCEPTION_CODE_INBOUND_GROUP_SESSION_DESERIALIZATION = 13;

    public static final int EXCEPTION_CODE_ACCOUNT_IDENTITY_KEYS = 20;
    public static final int EXCEPTION_CODE_ACCOUNT_GENERATE_ONE_TIME_KEYS = 21;
    public static final int EXCEPTION_CODE_ACCOUNT_ONE_TIME_KEYS = 22;
    public static final int EXCEPTION_CODE_ACCOUNT_REMOVE_ONE_TIME_KEYS = 23;
    public static final int EXCEPTION_CODE_ACCOUNT_MARK_ONE_KEYS_AS_PUBLISHED = 24;
    public static final int EXCEPTION_CODE_ACCOUNT_SIGN_MESSAGE = 25;

    // exception human readable messages
    public static final String EXCEPTION_MSG_NEW_OUTBOUND_GROUP_SESSION = "createNewSession() failed";
    public static final String EXCEPTION_MSG_NEW_INBOUND_GROUP_SESSION = "createNewSession() failed";
    public static final String EXCEPTION_MSG_INIT_OUTBOUND_GROUP_SESSION = "initOutboundGroupSession() failed";
    public static final String EXCEPTION_MSG_INIT_INBOUND_GROUP_SESSION = " initInboundGroupSessionWithSessionKey() failed";
    public static final String EXCEPTION_MSG_INIT_NEW_ACCOUNT_DESERIALIZATION = "initNewAccount() failure";
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
