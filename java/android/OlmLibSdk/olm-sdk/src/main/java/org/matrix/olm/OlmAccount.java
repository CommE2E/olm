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

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

public class OlmAccount {
    private static final String LOG_TAG = "OlmAccount";

    // JSON keys used in the JSON objects returned by JNI
    public static final String JSON_KEY_ONE_TIME_KEY = "curve25519";
    public static final String JSON_KEY_IDENTITY_KEY = "curve25519";
    public static final String JSON_KEY_FINGER_PRINT_KEY = "ed25519";

    /** instance unique identifier, used in JNI to match the corresponding native class **/
    private int mJavaInstanceId;

    /** account raw pointer value (OlmAccount*) returned by JNI.
     * this value identifies uniquely the native account instance.
     */
    private long mNativeOlmAccountId;

    public OlmAccount() {
        mJavaInstanceId = hashCode();
        //initNewAccount();
    }

    /**
     * Getter on the account ID.
     * @return native account ID
     */
    public long getOlmAccountId(){
        return mNativeOlmAccountId;
    }

    /**
     * Destroy the corresponding OLM account native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #initNewAccountJni()}.
     */
    private native void releaseAccountJni();

    /**
     * Release native account and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseAccountJni()}.
     * To be called before any other API call.
     */
    public void releaseAccount(){
        releaseAccountJni();

        mNativeOlmAccountId = 0;
    }

    /**
     * Create the corresponding OLM account in native side.<br>
     * The return value is a long casted C ptr on the OlmAccount.
     * Do not forget to call {@link #releaseAccount()} when JAVA side is done.
     * @return native account instance identifier (see {@link #mNativeOlmAccountId})
     */
    private native long initNewAccountJni();

    /**
     * Create and save the account native instance ID.
     * Wrapper for {@link #initNewAccountJni()}.<br>
     * To be called before any other API call.
     * @return true if init succeed, false otherwise.
     */
    public boolean initNewAccount() {
        boolean retCode = false;
        if(0 != (mNativeOlmAccountId = initNewAccountJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Get the public identity keys (Ed25519 fingerprint key and Curve25519 identity key).<br>
     * Keys are Base64 encoded.
     * These keys must be published on the server.
     * @return byte array containing the identity keys if operation succeed, null otherwise
     */
    private native byte[] identityKeysJni();

    /**
     * Return the identity keys (identity & fingerprint keys) in a JSON array.<br>
     * Public API for {@link #identityKeysJni()}.
     * @return identity keys in JSON array format if operation succeed, null otherwise
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
     * Return the largest number of "one time keys" this account can store.
     * @return the max number of "one time keys", -1 otherwise
     */
    public native long maxOneTimeKeys();

    /**
     * Generate a number of new one time keys.<br> If total number of keys stored
     * by this account exceeds {@link #maxOneTimeKeys()}, the old keys are discarded.
     * @param aNumberOfKeys number of keys to generate
     * @return 0 if operation succeed, -1 otherwise
     */
    public native int generateOneTimeKeys(int aNumberOfKeys);

    /**
     * Get the public parts of the unpublished "one time keys" for the account.<br>
     * The returned data is a JSON-formatted object with the single property
     * <tt>curve25519</tt>, which is itself an object mapping key id to
     * base64-encoded Curve25519 key.
     * These keys must be published on the server.
     * @return byte array containing the one time keys if operation succeed, null otherwise
     */
    private native byte[] oneTimeKeysJni();

    /**
     * Return the "one time keys" in a JSON array.<br>
     * Public API for {@link #oneTimeKeysJni()}.
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
     * Remove the "one time keys" that the session used from the account.
     * @param aNativeOlmSessionId native session instance identifier
     * @return 0 if operation succeed, 1 if no matching keys in the sessions to be removed, -1 if operation failed
     */
    public native int removeOneTimeKeysForSession(long aNativeOlmSessionId);

    /**
     * Marks the current set of "one time keys" as being published.
     * @return 0 if operation succeed, -1 otherwise
     */
    public native int markOneTimeKeysAsPublished();

    /**
     * Sign a message with the ed25519 fingerprint key for this account.
     * @param aMessage message to sign
     * @return the signed message if operation succeed, null otherwise
     */
    public native String signMessage(String aMessage);

    @Override
    public String toString() {
        return super.toString();
    }
}
