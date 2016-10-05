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

public class OlmSession {
    private static final String LOG_TAG = "OlmSession";
    /** session raw pointer value (OlmSession*) returned by JNI.
     * this value uniquely identifies the native session instance.
     **/
    private long mNativeOlmSessionId;

    /** account instance associated with this session. **/
    private OlmAccount mOlmAccount;

    public OlmSession() {
        //initNewSession();
    }

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
    public boolean initNewSession() {
        boolean retCode = false;
        if(0 != (mNativeOlmSessionId = initNewSessionJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create the corresponding OLM session in native side.<br>
     * The return value is a long casted C ptr on the OlmSession.
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

            int retCode = initOutboundSessionJni(mOlmAccount.getOlmAccountId(), aTheirIdentityKey, aTheirOneTimeKey);
            retObj = this;
        }

        return retObj;
    }

    private native int initOutboundSessionJni(long aOlmAccountId, String aTheirIdentityKey, String aTheirOneTimeKey);


    /**
     * Create a new in-bound session for sending/receiving messages from an
     * incoming PRE_KEY message.<br>
     * Public API for {@link #initInboundSessionJni(long, String)}.
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * @param aAccount the account to associate with this session
     * @param aOneTimeKeyMsg PRE KEY message TODO TBC
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
     * incoming PRE_KEY message based on the sender identity key TODO TBC!.<br>
     * Public API for {@link #initInboundSessionFromIdKeyJni(long, String, String)}.
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * @param aAccount the account to associate with this session
     * @param aTheirIdentityKey the sender identity key
     * @param aOneTimeKeyMsg PRE KEY message TODO TBC
     * @return this if operation succeed, null otherwise
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
     * Checks if the PRE_KEY message is for this in-bound session.<br>
     * This API may be used to process a "m.room.encrypted" event when type = 1 (PRE_KEY).
     * Public API for {@link #matchesInboundSessionJni(String)}.
     * @param aOneTimeKeyMsg PRE KEY message
     * @return this if operation succeed, null otherwise
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

/*
- (BOOL) matchesInboundSession:(NSString*)oneTimeKeyMessage;
- (BOOL) matchesInboundSessionFrom:(NSString*)theirIdentityKey oneTimeKeyMessage:(NSString *)oneTimeKeyMessage;

// UTF-8 plaintext -> base64 ciphertext
- (OLMMessage*) encryptMessage:(NSString*)message;

// base64 ciphertext -> UTF-8 plaintext
- (NSString*) decryptMessage:(OLMMessage*)message;
*/


    /**
     * Get the public identity keys (Ed25519 fingerprint key and Curve25519 identity key).<br>
     * Keys are Base64 encoded.
     * These keys must be published on the server.
     * @return byte array containing the identity keys if operation succeed, null otherwise
     */
    private native byte[] identityKeysJni();

    /**
     * Return the identity keys in a JSON array.<br>
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

