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

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Random;

public class OlmAccount implements Serializable {
    private static final long serialVersionUID = 3497486121598434824L;
    private static final String LOG_TAG = "OlmAccount";

    // JSON keys used in the JSON objects returned by JNI
    /** As well as the identity key, each device creates a number of Curve25519 key pairs which are
     also used to establish Olm sessions, but can only be used once. Once again, the private part
     remains on the device. but the public part is published to the Matrix network **/
    public static final String JSON_KEY_ONE_TIME_KEY = "curve25519";

    /** Curve25519 identity key is a public-key cryptographic system which can be used to establish a shared
     secret.<br>In Matrix, each device has a long-lived Curve25519 identity key which is used to establish
     Olm sessions with that device. The private key should never leave the device, but the
     public part is signed with the Ed25519 fingerprint key ({@link #JSON_KEY_FINGER_PRINT_KEY}) and published to the network. **/
    public static final String JSON_KEY_IDENTITY_KEY = "curve25519";

    /** Ed25519 finger print is a public-key cryptographic system for signing messages.<br>In Matrix, each device has
     an Ed25519 key pair which serves to identify that device. The private the key should
     never leave the device, but the public part is published to the Matrix network. **/
    public static final String JSON_KEY_FINGER_PRINT_KEY = "ed25519";

    /** account raw pointer value (OlmAccount*) returned by JNI.
     * this value identifies uniquely the native account instance.
     */
    private transient long mNativeOlmAccountId;


    public OlmAccount() throws OlmException {
        if(!initNewAccount()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_INIT_ACCOUNT_CREATION,OlmException.EXCEPTION_MSG_INIT_ACCOUNT_CREATION);
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
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_SERIALIZATION, String.valueOf(errorMsg));
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
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" key");

        } else if(TextUtils.isEmpty(pickledData)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" pickle");

        } else if(!createNewAccount()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, OlmException.EXCEPTION_MSG_INIT_NEW_ACCOUNT_DESERIALIZATION);

        } else if(!initWithSerializedData(pickledData, key, errorMsg)) {
            releaseAccount(); // prevent memory leak
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, String.valueOf(errorMsg));

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
     * Getter on the account ID.
     * @return native account ID
     */
    public long getOlmAccountId(){
        return mNativeOlmAccountId;
    }

    /**
     * Release native account and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseAccountJni()}.
     */
    public void releaseAccount(){
        releaseAccountJni();

        mNativeOlmAccountId = 0;
    }

    /**
     * Destroy the corresponding OLM account native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #initNewAccountJni()}.
     */
    private native void releaseAccountJni();

    /**
     * Create and initialize a native account instance.<br>
     * Wrapper for {@link #initNewAccountJni()}.
     * To be called before any other API call.
     * @return true if init succeed, false otherwise.
     */
    private boolean initNewAccount() {
        boolean retCode = false;
        if(0 != (mNativeOlmAccountId = initNewAccountJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create and initialize an OLM account in native side.<br>
     * Do not forget to call {@link #releaseAccount()} when JAVA side is done.
     * @return native account instance identifier (see {@link #mNativeOlmAccountId})
     */
    private native long initNewAccountJni();

    /**
     * Create a native account instance without any initialization.<br>
     * Since the account is left uninitialized, this
     * method is intended to be used in the serialization mechanism (see {@link #readObject(ObjectInputStream)}).<br>
     * Public wrapper for {@link #createNewAccountJni()}.
     * @return true if init succeed, false otherwise.
     */
    private boolean createNewAccount() {
        boolean retCode = false;
        if(0 != (mNativeOlmAccountId = createNewAccountJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create an OLM account in native side.<br>
     * Do not forget to call {@link #releaseAccount()} when JAVA side is done.
     * @return native account instance identifier (see {@link #mNativeOlmAccountId})
     */
    private native long createNewAccountJni();

    /**
     * Return the identity keys (identity &amp fingerprint keys) in a JSON array.<br>
     * Public API for {@link #identityKeysJni()}.<br>
     * Ex:<tt>
     * {
     *  "curve25519":"Vam++zZPMqDQM6ANKpO/uAl5ViJSHxV9hd+b0/fwRAg",
     *  "ed25519":"+v8SOlOASFTMrX3MCKBM4iVnYoZ+JIjpNt1fi8Z9O2I"
     * }</tt>
     * @return identity keys in JSON array if operation succeed, null otherwise
     */
    public JSONObject identityKeys() {
        JSONObject identityKeysJsonObj = null;
        byte identityKeysBuffer[];

        if( null != (identityKeysBuffer = identityKeysJni())) {
            try {
                identityKeysJsonObj = new JSONObject(new String(identityKeysBuffer));
                Log.d(LOG_TAG, "## identityKeys(): Identity Json keys=" + identityKeysJsonObj.toString());
            } catch (JSONException e) {
                identityKeysJsonObj = null;
                Log.e(LOG_TAG, "## identityKeys(): Exception - Msg=" + e.getMessage());
            }
        } else {
            Log.e(LOG_TAG, "## identityKeys(): Failure - identityKeysJni()=null");
        }

        return identityKeysJsonObj;
    }
    /**
     * Get the public identity keys (Ed25519 fingerprint key and Curve25519 identity key).<br>
     * Keys are Base64 encoded.
     * These keys must be published on the server.
     * @return byte array containing the identity keys if operation succeed, null otherwise
     */
    private native byte[] identityKeysJni();

    /**
     * Return the largest number of "one time keys" this account can store.
     * @return the max number of "one time keys", -1 otherwise
     */
    public native long maxOneTimeKeys();

    /**
     * Generate a number of new one time keys.<br> If total number of keys stored
     * by this account exceeds {@link #maxOneTimeKeys()}, the old keys are discarded.<br>
     * The corresponding keys are retrieved by {@link #oneTimeKeys()}.
     * @param aNumberOfKeys number of keys to generate
     * @return 0 if operation succeed, -1 otherwise
     */
    public native int generateOneTimeKeys(int aNumberOfKeys);

    /**
     * Return the "one time keys" in a JSON array.<br>
     * The number of "one time keys", is specified by {@link #generateOneTimeKeys(int)}<br>
     * Ex:<tt>
     * { "curve25519":
     *  {
     *      "AAAABQ":"qefVZd8qvjOpsFzoKSAdfUnJVkIreyxWFlipCHjSQQg",
     *      "AAAABA":"/X8szMU+p+lsTnr56wKjaLgjTMQQkCk8EIWEAilZtQ8",
     *      "AAAAAw":"qxNxxFHzevFntaaPdT0fhhO7tc7pco4+xB/5VRG81hA",
     *  }
     * }</tt><br>
     * Public API for {@link #oneTimeKeysJni()}.<br>
     * Note: these keys are to be published on the server.
     * @return one time keys in JSON array format if operation succeed, null otherwise
     */
    public JSONObject oneTimeKeys() {
        byte identityKeysBuffer[];
        JSONObject identityKeysJsonObj = null;

        if( null != (identityKeysBuffer = oneTimeKeysJni())) {
            try {
                identityKeysJsonObj = new JSONObject(new String(identityKeysBuffer));
                Log.d(LOG_TAG, "## oneTimeKeys(): Identity Json keys=" + identityKeysJsonObj.toString());
            } catch (JSONException e) {
                identityKeysJsonObj = null;
                Log.e(LOG_TAG, "## oneTimeKeys(): Exception - Msg=" + e.getMessage());
            }
        } else {
            Log.e(LOG_TAG, "## oneTimeKeys(): Failure - identityKeysJni()=null");
        }

        return identityKeysJsonObj;
    }
    /**
     * Get the public parts of the unpublished "one time keys" for the account.<br>
     * The returned data is a JSON-formatted object with the single property
     * <tt>curve25519</tt>, which is itself an object mapping key id to
     * base64-encoded Curve25519 key.<br>
     * @return byte array containing the one time keys if operation succeed, null otherwise
     */
    private native byte[] oneTimeKeysJni();

    /**
     * Remove the "one time keys" that the session used from the account.
     * @param aSession session instance
     * @return 0 if operation succeed, 1 if no matching keys in the sessions to be removed, -1 if operation failed
     */
    public int removeOneTimeKeysForSession(OlmSession aSession) {
        int retCode = 0;

        if(null != aSession) {
            retCode = removeOneTimeKeysForSessionJni(aSession.getOlmSessionId());
            Log.d(LOG_TAG,"## removeOneTimeKeysForSession(): result="+retCode);
        }

        return retCode;
    }
    /**
     * Remove the "one time keys" that the session used from the account.
     * @param aNativeOlmSessionId native session instance identifier
     * @return 0 if operation succeed, 1 if no matching keys in the sessions to be removed, -1 if operation failed
     */
    private native int removeOneTimeKeysForSessionJni(long aNativeOlmSessionId);

    /**
     * Marks the current set of "one time keys" as being published.
     * @return 0 if operation succeed, -1 otherwise
     */
    public int markOneTimeKeysAsPublished() {
        return markOneTimeKeysAsPublishedJni();
    }
    private native int markOneTimeKeysAsPublishedJni();

    /**
     * Sign a message with the ed25519 fingerprint key for this account.
     * The signed message is returned by the method.
     * @param aMessage message to sign
     * @return the signed message if operation succeed, null otherwise
     */
    public String signMessage(String aMessage) {
        return signMessageJni(aMessage);
    }
    private native String signMessageJni(String aMessage);
}
