/*
 * Copyright 2018 New Vector Ltd
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
import android.util.Log;

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmPkTest {
    private static final String LOG_TAG = "OlmPkEncryptionTest";

    private static OlmPkEncryption mOlmPkEncryption;
    private static OlmPkDecryption mOlmPkDecryption;

    @Test
    public void test01EncryptAndDecrypt() {
        try {
            mOlmPkEncryption = new OlmPkEncryption();
        } catch (OlmException e) {
            e.printStackTrace();
            assertTrue("OlmPkEncryption failed " + e.getMessage(), false);
        }
        try {
            mOlmPkDecryption = new OlmPkDecryption();
        } catch (OlmException e) {
            e.printStackTrace();
            assertTrue("OlmPkEncryption failed " + e.getMessage(), false);
        }

        assertNotNull(mOlmPkEncryption);
        assertNotNull(mOlmPkDecryption);

        String key = null;
        try {
            key = mOlmPkDecryption.generateKey();
        } catch (OlmException e) {
            assertTrue("Exception in generateKey, Exception code=" + e.getExceptionCode(), false);
        }
        Log.d(LOG_TAG, "Ephemeral Key: " + key);
        try {
            mOlmPkEncryption.setRecipientKey(key);
        } catch (OlmException e) {
            assertTrue("Exception in setRecipientKey, Exception code=" + e.getExceptionCode(), false);
        }

        String clearMessage = "Public key test";
        OlmPkMessage message = null;
        try {
            message = mOlmPkEncryption.encrypt(clearMessage);
        } catch (OlmException e) {
            assertTrue("Exception in encrypt, Exception code=" + e.getExceptionCode(), false);
        }
        Log.d(LOG_TAG, "message: " + message.mCipherText + " " + message.mMac + " " + message.mEphemeralKey);

        String decryptedMessage = null;
        try {
            decryptedMessage = mOlmPkDecryption.decrypt(message);
        } catch (OlmException e) {
            assertTrue("Exception in decrypt, Exception code=" + e.getExceptionCode(), false);
        }
        assertTrue(clearMessage.equals(decryptedMessage));

        mOlmPkEncryption.releaseEncryption();
        mOlmPkDecryption.releaseDecryption();
        assertTrue(mOlmPkEncryption.isReleased());
        assertTrue(mOlmPkDecryption.isReleased());
    }
}
