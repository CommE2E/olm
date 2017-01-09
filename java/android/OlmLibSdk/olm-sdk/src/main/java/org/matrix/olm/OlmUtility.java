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

import android.text.TextUtils;
import android.util.Log;

import java.security.SecureRandom;

/**
 * Olm SDK helper class.
 */
public class OlmUtility {
    private static final String LOG_TAG = "OlmUtility";

    public static final int RANDOM_KEY_SIZE = 32;

    /** Instance Id returned by JNI.
     * This value uniquely identifies this utility instance.
     **/
    private long mNativeId;

    public OlmUtility() throws OlmException  {
        initUtility();
    }

    /**
     * Create a native utility instance.
     * To be called before any other API call.
     * @exception OlmException the exception
     */
    private void initUtility() throws OlmException {
        try {
            mNativeId = createUtilityJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_UTILITY_CREATION, e.getMessage());
        }
    }

    private native long createUtilityJni();

    /**
     * Release native instance.<br>
     * Public API for {@link #releaseUtilityJni()}.
     */
    public void releaseUtility() {
        if (0 != mNativeId) {
            releaseUtilityJni();
        }
        mNativeId = 0;
    }
    private native void releaseUtilityJni();

    /**
     * Verify an ed25519 signature.<br>
     * If the signature is verified, the method returns true. If false is returned, an error description is provided in aError.
     * If the key was too small, aError is set to "OLM.INVALID_BASE64".
     * If the signature was invalid, aError is set to "OLM.BAD_MESSAGE_MAC".<br>
     * @param aSignature the base64-encoded message signature to be checked.
     * @param aFingerprintKey the ed25519 key (fingerprint key)
     * @param aMessage the signed message
     * @return true if the signature is verified, false otherwise
     * @exception OlmException the failure reason
     */
    public boolean verifyEd25519Signature(String aSignature, String aFingerprintKey, String aMessage) throws OlmException {
        String errorMessage = null;

        try {
            if (TextUtils.isEmpty(aSignature) || TextUtils.isEmpty(aFingerprintKey) || TextUtils.isEmpty(aMessage)) {
                Log.e(LOG_TAG, "## verifyEd25519Signature(): invalid input parameters");
                errorMessage = "JAVA sanity check failure - invalid input parameters";
            } else {
                errorMessage =  verifyEd25519SignatureJni(aSignature.getBytes("UTF-8"), aFingerprintKey.getBytes("UTF-8"), aMessage.getBytes("UTF-8"));
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "## verifyEd25519Signature(): failed " + e.getMessage());
        }

        if (!TextUtils.isEmpty(errorMessage)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_UTILITY_VERIFY_SIGNATURE, errorMessage);
        }

        return true;
    }

    /**
     * Verify an ed25519 signature.
     * Return a human readable error message in case of verification failure.
     * @param aSignature the base64-encoded message signature to be checked.
     * @param aFingerprintKey the ed25519 key
     * @param aMessage the signed message
     * @return null if validation succeed, the error message string if operation failed
     */
    private native String verifyEd25519SignatureJni(byte[] aSignature, byte[] aFingerprintKey, byte[] aMessage);


    /**
     * Compute the hash(SHA-256) value of the string given in parameter(aMessageToHash).<br>
     * The hash value is the returned by the method.
     * @param aMessageToHash message to be hashed
     * @return hash value if operation succeed, null otherwise
     */
     public String sha256(String aMessageToHash) {
         String hashRetValue = null;

         if (null != aMessageToHash) {
             try {
                 hashRetValue = new String(sha256Jni(aMessageToHash.getBytes("UTF-8")), "UTF-8");
             } catch (Exception e) {
                 Log.e(LOG_TAG, "## sha256(): failed " + e.getMessage());
             }
         }

        return hashRetValue;
    }

    private native byte[] sha256Jni(byte[] aMessage);


    /**
     * Helper method to compute a string based on random integers.
     * @return bytes buffer containing randoms integer values
     */
    public static byte[] getRandomKey() {
        SecureRandom secureRandom = new SecureRandom();
        byte[] buffer = new byte[RANDOM_KEY_SIZE];
        secureRandom.nextBytes(buffer);

        // the key is saved as string
        // so avoid the UTF8 marker bytes
        for(int i = 0; i < RANDOM_KEY_SIZE; i++) {
            buffer[i] = (byte)(buffer[i] & 0x7F);
        }
        return buffer;
    }

    /**
     * Return true the object resources have been released.<br>
     * @return true the object resources have been released
     */
    public boolean isReleased() {
        return (0 == mNativeId);
    }
}

