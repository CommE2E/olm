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

public class OlmOutboundGroupSession implements Serializable {
    private static final long serialVersionUID = -3133097431283604416L;
    private static final String LOG_TAG = "OlmOutboundGroupSession";

    /** session raw pointer value returned by JNI.<br>
     * this value uniquely identifies the native inbound group session instance.
     */
    private long mNativeOlmOutboundGroupSessionId;

    /**
     * Getter on the native outbound group session ID.
     * @return native outbound group session ID
     */
    public long getOlmInboundGroupSessionId(){
        return mNativeOlmOutboundGroupSessionId;
    }

    /**
     * Constructor.<br>
     * Create and save a new session native instance ID and
     * initialise a new outbound group session.<br>
     * See {@link #initNewSession()} and {@link #initOutboundGroupSession()}
     * @throws OlmException constructor failure
     */
    public OlmOutboundGroupSession() throws OlmException {
        if(initNewSession()) {
            if( 0 != initOutboundGroupSession()) {
                releaseSession();// prevent memory leak before throwing
                throw new OlmException(OlmException.EXCEPTION_CODE_INIT_OUTBOUND_GROUP_SESSION, OlmException.EXCEPTION_MSG_INIT_OUTBOUND_GROUP_SESSION);
            }
        } else {
            throw new OlmException(OlmException.EXCEPTION_CODE_CREATE_OUTBOUND_GROUP_SESSION, OlmException.EXCEPTION_MSG_NEW_OUTBOUND_GROUP_SESSION);
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
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_SERIALIZATION, String.valueOf(errorMsg));
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
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" key");

        } else if(TextUtils.isEmpty(pickledData)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" pickle");

        } else if(!initNewSession()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_DESERIALIZATION, OlmException.EXCEPTION_MSG_INIT_NEW_ACCOUNT_DESERIALIZATION);

        } else if(!initWithSerializedData(pickledData, key, errorMsg)) {
            releaseSession(); // prevent memory leak
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_DESERIALIZATION, String.valueOf(errorMsg));

        } else {
            Log.d(LOG_TAG,"## readObject(): success");
        }
    }

    /**
     * Return a session as a base64 string.<br>
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
     * Release native session and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseSessionJni()}.
     */
    public void releaseSession() {
        releaseSessionJni();        
        mNativeOlmOutboundGroupSessionId = 0;
    }

    /**
     * Destroy the corresponding OLM outbound group session native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #initNewSessionJni()}.
     */
    private native void releaseSessionJni();

    /**
     * Create and save the session native instance ID.
     * Wrapper for {@link #initNewSessionJni()}.<br>
     * To be called before any other API call.
     * @return true if init succeed, false otherwise.
     */
    private boolean initNewSession() {
        boolean retCode = false;
        if(0 != (mNativeOlmOutboundGroupSessionId = initNewSessionJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create the corresponding OLM outbound group session in native side.<br>
     * Do not forget to call {@link #releaseSession()} when JAVA side is done.
     * @return native session instance identifier (see {@link #mNativeOlmOutboundGroupSessionId})
     */
    private native long initNewSessionJni();

    /**
     * Start a new outbound group session.<br>
     * @return 0 if operation succeed, -1 otherwise
     */
    private int initOutboundGroupSession() {
        return initOutboundGroupSessionJni();
    }
    private native int initOutboundGroupSessionJni();

    /**
     * Get a base64-encoded identifier for this session.
     * @return session identifier if operation succeed, null otherwise.
     */
    public String sessionIdentifier() {
        String retValue = null;
        retValue = sessionIdentifierJni();

        return retValue;
    }
    private native String sessionIdentifierJni();

    /**
     * Get the current message index for this session.<br>
     * Each message is sent with an increasing index, this
     * method returns the index for the next message.
     * @return current session index
     */
    public int messageIndex() {
        int retValue =0;
        retValue = messageIndexJni();

        return retValue;
    }
    private native int messageIndexJni();

    /**
     * Get the base64-encoded current ratchet key for this session.<br>
     * Each message is sent with a different ratchet key. This method returns the
     * ratchet key that will be used for the next message.
     * @return outbound session key
     */
    public String sessionKey() {
        String retValue = null;
        retValue = sessionKeyJni();

        return retValue;
    }
    private native String sessionKeyJni();

    /**
     * Encrypt some plain-text message.<br>
     * @param aClearMsg message to be encrypted
     * @return the encrypted message if operation succeed, null otherwise
     */
    public String encryptMessage(String aClearMsg) {
        String retValue = null;

        if(!TextUtils.isEmpty(aClearMsg)) {
            retValue = encryptMessageJni(aClearMsg);
        }

        return retValue;
    }
    private native String encryptMessageJni(String aClearMsg);
}
