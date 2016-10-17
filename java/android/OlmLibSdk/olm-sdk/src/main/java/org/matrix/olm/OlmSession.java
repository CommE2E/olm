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

import android.text.TextUtils;
import android.util.Log;

import java.io.Serializable;

public class OlmSession implements Serializable {
    private static final String LOG_TAG = "OlmSession";

    /** session raw pointer value (OlmSession*) returned by JNI.
     * this value uniquely identifies the native session instance.
     **/
    private long mNativeOlmSessionId;

    /** account instance associated with this session. **/
    private OlmAccount mOlmAccount;

    public OlmSession() {
        initNewSession();
    }

    /**
     * Getter on the session ID.
     * @return native session ID
     */
    public long getOlmSessionId(){
        return mNativeOlmSessionId;
    }

    /**
     * Getter on the session ID.
     * @return native session ID
     */
    public OlmAccount getOlmAccountId(){
        return mOlmAccount;
    }

    /**
     * Destroy the corresponding OLM session native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #initNewSessionJni()}.
     */
    private native void releaseSessionJni();

    /**
     * Release native session and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseSessionJni()}.
     * To be called before any other API call.
     */
    public void releaseSession(){
        releaseSessionJni();

        mNativeOlmSessionId = 0;
    }

    /**
     * Create and save the session native instance ID.
     * Wrapper for {@link #initNewSessionJni()}.<br>
     * To be called before any other API call.
     * @return true if init succeed, false otherwise.
     */
    private boolean initNewSession() {
        boolean retCode = false;
        if(0 != (mNativeOlmSessionId = initNewSessionJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create the corresponding OLM session in native side.<br>
     * Do not forget to call {@link #releaseSession()} when JAVA side is done.
     * @return native session instance identifier (see {@link #mNativeOlmSessionId})
     */
    private native long initNewSessionJni();


    /**
     * Creates a new out-bound session for sending messages to a recipient
     * identified by an identity key and a one time key.<br>
     * Public API for {@link #initOutboundSessionWithAccount(OlmAccount, String, String)}.
     * @param aAccount the account to associate with this session
     * @param aTheirIdentityKey the identity key of the recipient
     * @param aTheirOneTimeKey the one time key of the recipient
     * @return this if operation succeed, null otherwise
     */
    public OlmSession initOutboundSessionWithAccount(OlmAccount aAccount, String aTheirIdentityKey, String aTheirOneTimeKey) {
        OlmSession retObj=null;

        if((null==aAccount) || TextUtils.isEmpty(aTheirIdentityKey) || TextUtils.isEmpty(aTheirOneTimeKey)){
            Log.e(LOG_TAG, "## initOutboundSession(): invalid input parameters");
        } else {
            // set the account of this session
            mOlmAccount = aAccount;

            if(0 == initOutboundSessionJni(mOlmAccount.getOlmAccountId(), aTheirIdentityKey, aTheirOneTimeKey)) {
                retObj = this;
            }
        }

        return retObj;
    }

    private native int initOutboundSessionJni(long aOlmAccountId, String aTheirIdentityKey, String aTheirOneTimeKey);


    /**
     * Create a new in-bound session for sending/receiving messages from an
     * incoming PRE_KEY ({@link OlmMessage#MESSAGE_TYPE_PRE_KEY}) message.<br>
     * Public API for {@link #initInboundSessionJni(long, String)}.
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * @param aAccount the account to associate with this session
     * @param aOneTimeKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
     */
    public OlmSession initInboundSessionWithAccount(OlmAccount aAccount, String aOneTimeKeyMsg) {
        OlmSession retObj=null;

        if((null==aAccount) || TextUtils.isEmpty(aOneTimeKeyMsg)){
            Log.e(LOG_TAG, "## initInboundSessionWithAccount(): invalid input parameters");
        } else {
            // set the account of this session
            mOlmAccount = aAccount;

            if( 0 == initInboundSessionJni(mOlmAccount.getOlmAccountId(), aOneTimeKeyMsg)) {
                retObj = this;
            }
        }

        return retObj;
    }

    private native int initInboundSessionJni(long aOlmAccountId, String aOneTimeKeyMsg);


    /**
     * Create a new in-bound session for sending/receiving messages from an
     * incoming PRE_KEY({@link OlmMessage#MESSAGE_TYPE_PRE_KEY}) message based on the sender identity key.<br>
     * Public API for {@link #initInboundSessionFromIdKeyJni(long, String, String)}.
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * @param aAccount the account to associate with this session
     * @param aTheirIdentityKey the sender identity key
     * @param aOneTimeKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
     * TODO unit test missing: initInboundSessionWithAccountFrom
     */
    public OlmSession initInboundSessionWithAccountFrom(OlmAccount aAccount, String aTheirIdentityKey, String aOneTimeKeyMsg) {
        OlmSession retObj=null;

        if((null==aAccount) || TextUtils.isEmpty(aOneTimeKeyMsg)){
            Log.e(LOG_TAG, "## initInboundSessionWithAccount(): invalid input parameters");
        } else {
            // set the account of this session
            mOlmAccount = aAccount;

            if(0 == initInboundSessionFromIdKeyJni(mOlmAccount.getOlmAccountId(), aTheirIdentityKey, aOneTimeKeyMsg)){
                retObj = this;
            }
        }

        return retObj;
    }

    private native int initInboundSessionFromIdKeyJni(long aOlmAccountId, String aTheirIdentityKey, String aOneTimeKeyMsg);

    /**
     * Get the session identifier.<br> Will be the same for both ends of the
     * conversation. The session identifier is returned as a String object.
     * Session Id sample: "session_id":"M4fOVwD6AABrkTKl"
     * Public API for {@link #getSessionIdentifierJni()}.
     * @return the session ID as a String if operation succeed, null otherwise
     */
    public String sessionIdentifier() {
        return getSessionIdentifierJni();
    }

    private native String getSessionIdentifierJni();

    /**
     * Checks if the PRE_KEY({@link OlmMessage#MESSAGE_TYPE_PRE_KEY}) message is for this in-bound session.<br>
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * Public API for {@link #matchesInboundSessionJni(String)}.
     * @param aOneTimeKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
     * TODO unit test missing: matchesInboundSession
     */
    public boolean matchesInboundSession(String aOneTimeKeyMsg) {
        boolean retCode = false;

        if(0 == matchesInboundSessionJni(aOneTimeKeyMsg)){
            retCode = true;
        }
        return retCode;
    }

    private native int matchesInboundSessionJni(String aOneTimeKeyMsg);


    /**
     * Checks if the PRE_KEY({@link OlmMessage#MESSAGE_TYPE_PRE_KEY}) message is for this in-bound session based on the sender identity key.<br>
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * Public API for {@link #matchesInboundSessionJni(String)}.
     * @param aTheirIdentityKey the sender identity key
     * @param aOneTimeKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
     * TODO unit test missing: matchesInboundSessionFrom
     */
    public boolean matchesInboundSessionFrom(String aTheirIdentityKey, String aOneTimeKeyMsg) {
        boolean retCode = false;

        if(0 == matchesInboundSessionFromIdKeyJni(aTheirIdentityKey, aOneTimeKeyMsg)){
            retCode = true;
        }
        return retCode;
    }

    private native int matchesInboundSessionFromIdKeyJni(String aTheirIdentityKey, String aOneTimeKeyMsg);


    /**
     * Encrypt a message using the session.<br>
     * The encrypted message is returned in a OlmMessage object.
     * Public API for {@link #encryptMessageJni(String, OlmMessage)}.
     * @param aClearMsg message to encrypted
     * @return the encrypted message if operation succeed, null otherwise
     */
    public OlmMessage encryptMessage(String aClearMsg) {
        OlmMessage encryptedMsgRetValue = new OlmMessage();

        if(0 != encryptMessageJni(aClearMsg, encryptedMsgRetValue)){
            encryptedMsgRetValue = null;
        }

        return encryptedMsgRetValue;
    }

    private native int encryptMessageJni(String aClearMsg, OlmMessage aEncryptedMsg);

    /**
     * Decrypt a message using the session.<br>
     * The encrypted message is given as a OlmMessage object.
     * @param aEncryptedMsg message to decrypt
     * @return the decrypted message if operation succeed, null otherwise
     */
    public String decryptMessage(OlmMessage aEncryptedMsg) {
        return decryptMessageJni(aEncryptedMsg);
    }

    private native String decryptMessageJni(OlmMessage aEncryptedMsg);

    // TODO missing API: initWithSerializedData
    // TODO missing API: serializeDataWithKey
    // TODO missing API: initWithCoder
    // TODO missing API: encodeWithCoder
}

