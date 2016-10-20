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

/**
 * Exception class to identify specific Olm SDk exceptions.
 */
public class OlmException extends Exception {
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

    // exception human readable messages
    public static final String EXCEPTION_MSG_NEW_OUTBOUND_GROUP_SESSION = "failed to create a new outbound group Session";
    public static final String EXCEPTION_MSG_NEW_INBOUND_GROUP_SESSION = "failed to create a new inbound group Session";
    public static final String EXCEPTION_MSG_INIT_OUTBOUND_GROUP_SESSION = "failed to initialize a new outbound group Session";
    public static final String EXCEPTION_MSG_INIT_INBOUND_GROUP_SESSION = "failed to initialize a new inbound group Session";
    public static final String EXCEPTION_MSG_INIT_NEW_ACCOUNT_DESERIALIZATION = "initNewAccount() failure";
    public static final String EXCEPTION_MSG_INIT_ACCOUNT_DESERIALIZATION = "initWithSerializedData() failure";
    public static final String EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION = "invalid deserialized parameters";
    public static final String EXCEPTION_MSG_INIT_ACCOUNT_CREATION = "Account constructor failure";

    /** exception code to be taken from: {@link #EXCEPTION_CODE_CREATE_OUTBOUND_GROUP_SESSION} {@link #EXCEPTION_CODE_CREATE_INBOUND_GROUP_SESSION}
     * {@link #EXCEPTION_CODE_INIT_OUTBOUND_GROUP_SESSION} {@link #EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION}**/
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
