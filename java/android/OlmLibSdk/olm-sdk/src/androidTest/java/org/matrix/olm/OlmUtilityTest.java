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

import android.support.test.runner.AndroidJUnit4;
import android.text.TextUtils;
import android.util.Log;

import org.json.JSONObject;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmUtilityTest {
    private static final String LOG_TAG = "OlmAccountTest";
    private static final int GENERATION_ONE_TIME_KEYS_NUMBER = 50;

    private static OlmManager mOlmManager;

    @BeforeClass
    public static void setUpClass(){
        // load native lib
        mOlmManager = new OlmManager();

        String version = mOlmManager.getOlmLibVersion();
        assertNotNull(version);
        Log.d(LOG_TAG, "## setUpClass(): lib version="+version);
    }

    /**
     * Test the signing API
     */
    @Test
    public void test01VerifyEd25519Signing() {
        String fingerPrintKey = null;
        StringBuffer errorMsg = new StringBuffer();
        String message = "{\"key1\":\"value1\",\"key2\":\"value2\"};";
        OlmAccount account = null;

        // create account
        try {
            account = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }
        assertNotNull(account);

        // sign message
        String messageSignature = account.signMessage(message);
        assertNotNull(messageSignature);

        // get identities key (finger print key)
        JSONObject identityKeysJson = account.identityKeys();
        assertNotNull(identityKeysJson);
        fingerPrintKey = TestHelper.getFingerprintKey(identityKeysJson);
        assertTrue("fingerprint key missing",!TextUtils.isEmpty(fingerPrintKey));

        // instantiate utility object
        OlmUtility utility = new OlmUtility();

        // verify signature
        errorMsg.append("init with anything");
        boolean isVerified = utility.verifyEd25519Signature(messageSignature, fingerPrintKey, message, errorMsg);
        assertTrue(isVerified);
        assertTrue(String.valueOf(errorMsg).isEmpty());

        // check a bad signature is detected => errorMsg = BAD_MESSAGE_MAC
        String badSignature = "Bad signature Bad signature Bad signature..";
        isVerified = utility.verifyEd25519Signature(badSignature, fingerPrintKey, message, errorMsg);
        assertFalse(isVerified);
        assertFalse(String.valueOf(errorMsg).isEmpty());

        // check bad fingerprint size => errorMsg = INVALID_BASE64
        String badSizeFingerPrintKey = fingerPrintKey.substring(fingerPrintKey.length()/2);
        isVerified = utility.verifyEd25519Signature(messageSignature, badSizeFingerPrintKey, message, errorMsg);
        assertFalse(isVerified);
        assertFalse(String.valueOf(errorMsg).isEmpty());

        utility.releaseUtility();
        account.releaseAccount();
    }


    @Test
    public void test02sha256() {
        OlmUtility utility = new OlmUtility();
        String msgToHash = "The quick brown fox jumps over the lazy dog";

        String hashResult = utility.sha256(msgToHash);
        assertFalse(TextUtils.isEmpty(hashResult));

        utility.releaseUtility();
    }
}
