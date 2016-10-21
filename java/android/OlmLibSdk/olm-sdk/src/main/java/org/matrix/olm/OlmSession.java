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

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

public class OlmSession implements Serializable {
    private static final long serialVersionUID = -8975488639186976419L;
    private static final String LOG_TAG = "OlmSession";

    /** session raw pointer value (OlmSession*) returned by JNI.
     * this value uniquely identifies the native session instance.
     **/
    private transient long mNativeOlmSessionId;


    public OlmSession() throws OlmException {
        if(!initNewSession()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_INIT_SESSION_CREATION, OlmException.EXCEPTION_MSG_INIT_SESSION_CREATION);
        }
    }

    /**
     * Kick off the serialization mechanism.
     * @param aOutStream output stream for serializing
     * @throws IOException
     * @throws OlmException
     */
    private void writeObject(ObjectOutputStream aOutStream) throws IOException, OlmException {
        aOutStream.defaultWriteObject();

        // generate serialization key
        String key = OlmUtility.getRandomKey();

        // compute pickle string
        StringBuffer errorMsg = new StringBuffer();
        String pickledData = serializeDataWithKey(key, errorMsg);

        if(null == pickledData) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SESSION_SERIALIZATION, String.valueOf(errorMsg));
        } else {
            aOutStream.writeObject(key);
            aOutStream.writeObject(pickledData);
        }
    }

    /**
     * Kick off the deserialization mechanism.
     * @param aInStream
     * @throws IOException
     * @throws ClassNotFoundException
     * @throws OlmException
     */
    private void readObject(ObjectInputStream aInStream) throws IOException, ClassNotFoundException, OlmException {
        aInStream.defaultReadObject();
        StringBuffer errorMsg = new StringBuffer();

        String key = (String) aInStream.readObject();
        String pickledData = (String) aInStream.readObject();

        if(TextUtils.isEmpty(key)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SESSION_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" key");

        } else if(TextUtils.isEmpty(pickledData)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SESSION_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" pickle");

        } else if(!createNewSession()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SESSION_DESERIALIZATION, OlmException.EXCEPTION_MSG_INIT_NEW_ACCOUNT_DESERIALIZATION);

        } else if(!initWithSerializedData(pickledData, key, errorMsg)) {
            releaseSession(); // prevent memory leak
            throw new OlmException(OlmException.EXCEPTION_CODE_SESSION_DESERIALIZATION, String.valueOf(errorMsg));

        } else {
            Log.d(LOG_TAG,"## readObject(): success");
        }
    }

    /**
     * Return an account as a base64 string.<br>
     * The account is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return pickled base64 string if operation succeed, null otherwise
     */
    private String serializeDataWithKey(String aKey, StringBuffer aErrorMsg) {
        String pickleRetValue = null;

        // sanity check
        if(null == aErrorMsg) {
            Log.e(LOG_TAG,"## serializeDataWithKey(): invalid parameter - aErrorMsg=null");
        } else if(TextUtils.isEmpty(aKey)) {
            aErrorMsg.append("Invalid input parameters in serializeDataWithKey()");
        } else {
            aErrorMsg.setLength(0);
            pickleRetValue = serializeDataWithKeyJni(aKey, aErrorMsg);
        }

        return pickleRetValue;
    }
    private native String serializeDataWithKeyJni(String aKey, StringBuffer aErrorMsg);


    /**
     * Loads an account from a pickled base64 string.<br>
     * See {@link #serializeDataWithKey(String, StringBuffer)}
     * @param aSerializedData pickled account in a base64 string format
     * @param aKey key used to encrypted
     * @param aErrorMsg error message description
     * @return true if operation succeed, false otherwise
     */
    private boolean initWithSerializedData(String aSerializedData, String aKey, StringBuffer aErrorMsg) {
        boolean retCode = false;
        String jniError;

        if(null == aErrorMsg) {
            Log.e(LOG_TAG, "## initWithSerializedData(): invalid input error parameter");
        } else {
            aErrorMsg.setLength(0);

            if (TextUtils.isEmpty(aSerializedData) || TextUtils.isEmpty(aKey)) {
                Log.e(LOG_TAG, "## initWithSerializedData(): invalid input parameters");
            } else if (null == (jniError = initWithSerializedDataJni(aSerializedData, aKey))) {
                retCode = true;
            } else {
                aErrorMsg.append(jniError);
            }
        }

        return retCode;
    }
    private native String initWithSerializedDataJni(String aSerializedData, String aKey);

    /**
     * Getter on the session ID.
     * @return native session ID
     */
    public long getOlmSessionId(){
        return mNativeOlmSessionId;
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
     * Create a native account instance without any initialization.<br>
     * Since the account is left uninitialized, this
     * method is intended to be used in the serialization mechanism (see {@link #readObject(ObjectInputStream)}).<br>
     * Public wrapper for {@link #createNewSessionJni()}.
     * @return true if init succeed, false otherwise.
     */
    private boolean createNewSession() {
        boolean retCode = false;
        if(0 != (mNativeOlmSessionId = createNewSessionJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create an OLM account in native side.<br>
     * Do not forget to call {@link #releaseSession()} when JAVA side is done.
     * @return native account instance identifier (see {@link #mNativeOlmSessionId})
     */
    private native long createNewSessionJni();


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
            if(0 == initOutboundSessionJni(aAccount.getOlmAccountId(), aTheirIdentityKey, aTheirOneTimeKey)) {
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
     * @param aPreKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
     */
    public OlmSession initInboundSessionWithAccount(OlmAccount aAccount, String aPreKeyMsg) {
        OlmSession retObj=null;

        if((null==aAccount) || TextUtils.isEmpty(aPreKeyMsg)){
            Log.e(LOG_TAG, "## initInboundSessionWithAccount(): invalid input parameters");
        } else {
            if( 0 == initInboundSessionJni(aAccount.getOlmAccountId(), aPreKeyMsg)) {
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
     * This method must only be called the first time a pre-key message is received from an inbound session.
     * @param aAccount the account to associate with this session
     * @param aTheirIdentityKey the sender identity key
     * @param aPreKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
     * TODO unit test missing: initInboundSessionWithAccountFrom
     */
    public OlmSession initInboundSessionWithAccountFrom(OlmAccount aAccount, String aTheirIdentityKey, String aPreKeyMsg) {
        OlmSession retObj=null;

        if((null==aAccount) || TextUtils.isEmpty(aPreKeyMsg)){
            Log.e(LOG_TAG, "## initInboundSessionWithAccount(): invalid input parameters");
        } else {
            if(0 == initInboundSessionFromIdKeyJni(aAccount.getOlmAccountId(), aTheirIdentityKey, aPreKeyMsg)){
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
}

