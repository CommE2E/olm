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

import android.content.Context;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.json.JSONObject;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Map;

import static android.support.test.InstrumentationRegistry.getInstrumentation;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmSessionTest {
    private static final String LOG_TAG = "OlmSessionTest";
    private final String INVALID_PRE_KEY = "invalid PRE KEY hu hu!";
    private final String FILE_NAME_SERIAL_SESSION = "SerialSession";
    private final int ONE_TIME_KEYS_NUMBER = 4;

    private static OlmManager mOlmManager;

    @BeforeClass
    public static void setUpClass(){
        // enable UTF-8 specific conversion for pre Marshmallow(23) android versions,
        // due to issue described here: https://github.com/eclipsesource/J2V8/issues/142
        boolean isSpecificUtf8ConversionEnabled = android.os.Build.VERSION.SDK_INT < 23;

        // load native lib
        mOlmManager = new OlmManager(isSpecificUtf8ConversionEnabled);

        String version = mOlmManager.getOlmLibVersion();
        assertNotNull(version);
        Log.d(LOG_TAG, "## setUpClass(): lib version="+version);
    }

    /**
     * Basic test:
     * - alice creates an account
     * - bob creates an account
     * - alice creates an outbound session with bob (bobIdentityKey & bobOneTimeKey)
     * - alice encrypts a message with its session
     * - bob creates an inbound session based on alice's encrypted message
     * - bob decrypts the encrypted message with its session
     */
    @Test
    public void test01AliceToBob() {
        final int ONE_TIME_KEYS_NUMBER = 5;
        String bobIdentityKey = null;
        String bobOneTimeKey=null;
        OlmAccount bobAccount = null;
        OlmAccount aliceAccount = null;

        // ALICE & BOB ACCOUNTS CREATION
        try {
            aliceAccount = new OlmAccount();
            bobAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // get bob identity key
        Map<String, String> bobIdentityKeys = bobAccount.identityKeys();
        bobIdentityKey = TestHelper.getIdentityKey(bobIdentityKeys);
        assertTrue(null!=bobIdentityKey);

        // get bob one time keys
        assertTrue(0==bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        Map<String, Map<String, String>> bobOneTimeKeys = bobAccount.oneTimeKeys();
        bobOneTimeKey = TestHelper.getOneTimeKey(bobOneTimeKeys,1);
        assertNotNull(bobOneTimeKey);

        // CREATE ALICE SESSION
        OlmSession aliceSession = null;
        try {
            aliceSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE OUTBOUND SESSION and encrypt message to bob
        assertNotNull(aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));
        String clearMsg = "Heloo bob , this is alice!";
        OlmMessage encryptedMsgToBob = aliceSession.encryptMessage(clearMsg);
        assertNotNull(encryptedMsgToBob);
        assertNotNull(encryptedMsgToBob.mCipherText);
        Log.d(LOG_TAG,"## test01AliceToBob(): encryptedMsg="+encryptedMsgToBob.mCipherText);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=bobSession.getOlmSessionId());
        assertTrue(0==bobSession.initInboundSessionWithAccount(bobAccount, encryptedMsgToBob.mCipherText));
        String decryptedMsg = bobSession.decryptMessage(encryptedMsgToBob);
        assertNotNull(decryptedMsg);

        // MESSAGE COMPARISON: decrypted vs encrypted
        assertTrue(clearMsg.equals(decryptedMsg));

        // clean objects..
        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));

        // release accounts
        bobAccount.releaseAccount();
        aliceAccount.releaseAccount();
        assertTrue(0==bobAccount.getUnreleasedCount());
        assertTrue(0==aliceAccount.getUnreleasedCount());

        // release sessions
        bobSession.releaseSession();
        aliceSession.releaseSession();
        assertTrue(0==bobSession.getUnreleasedCount());
        assertTrue(0==aliceSession.getUnreleasedCount());
    }


    /**
     * Same as test01AliceToBob but with bob who's encrypting messages
     * to alice and alice decrypt them.<br>
     * - alice creates an account
     * - bob creates an account
     * - alice creates an outbound session with bob (bobIdentityKey & bobOneTimeKey)
     * - alice encrypts a message with its own session
     * - bob creates an inbound session based on alice's encrypted message
     * - bob decrypts the encrypted message with its own session
     * - bob encrypts messages with its own session
     * - alice decrypts bob's messages with its own message
     * - alice encrypts a message
     * - bob decrypts the encrypted message
     */
    @Test
    public void test02AliceToBobBackAndForth() {
        String bobIdentityKey;
        String bobOneTimeKey;
        OlmAccount aliceAccount = null;
        OlmAccount bobAccount = null;

        // creates alice & bob accounts
        try {
            aliceAccount = new OlmAccount();
            bobAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // get bob identity key
        Map<String, String> bobIdentityKeys = bobAccount.identityKeys();
        bobIdentityKey = TestHelper.getIdentityKey(bobIdentityKeys);
        assertTrue(null!=bobIdentityKey);

        // get bob one time keys
        assertTrue(0==bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        Map<String, Map<String, String>> bobOneTimeKeys = bobAccount.oneTimeKeys();
        bobOneTimeKey = TestHelper.getOneTimeKey(bobOneTimeKeys,1);
        assertNotNull(bobOneTimeKey);

        // CREATE ALICE SESSION
        OlmSession aliceSession = null;
        try {
            aliceSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE OUTBOUND SESSION and encrypt message to bob
        assertNotNull(aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));
        String helloClearMsg = "Hello I'm Alice!";

        OlmMessage encryptedAliceToBobMsg1 = aliceSession.encryptMessage(helloClearMsg);
        assertNotNull(encryptedAliceToBobMsg1);
        assertNotNull(encryptedAliceToBobMsg1.mCipherText);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=bobSession.getOlmSessionId());
        assertTrue(0==bobSession.initInboundSessionWithAccount(bobAccount, encryptedAliceToBobMsg1.mCipherText));

        // DECRYPT MESSAGE FROM ALICE
        String decryptedMsg01 = bobSession.decryptMessage(encryptedAliceToBobMsg1);
        assertNotNull(decryptedMsg01);

        // MESSAGE COMPARISON: decrypted vs encrypted
        assertTrue(helloClearMsg.equals(decryptedMsg01));

        // BACK/FORTH MESSAGE COMPARISON
        String clearMsg1 = "Hello I'm Bob!";
        String clearMsg2 = "Isn't life grand?";
        String clearMsg3 = "Let's go to the opera.";

        // bob encrypts messages
        OlmMessage encryptedMsg1 = bobSession.encryptMessage(clearMsg1);
        assertNotNull(encryptedMsg1);
        OlmMessage encryptedMsg2 = bobSession.encryptMessage(clearMsg2);
        assertNotNull(encryptedMsg2);
        OlmMessage encryptedMsg3 = bobSession.encryptMessage(clearMsg3);
        assertNotNull(encryptedMsg3);

        // alice decrypts bob's messages
        String decryptedMsg1 = aliceSession.decryptMessage(encryptedMsg1);
        assertNotNull(decryptedMsg1);
        String decryptedMsg2 = aliceSession.decryptMessage(encryptedMsg2);
        assertNotNull(decryptedMsg2);
        String decryptedMsg3 = aliceSession.decryptMessage(encryptedMsg3);
        assertNotNull(decryptedMsg3);

        // comparison tests
        assertTrue(clearMsg1.equals(decryptedMsg1));
        assertTrue(clearMsg2.equals(decryptedMsg2));
        assertTrue(clearMsg3.equals(decryptedMsg3));

        // and one more from alice to bob
        clearMsg1 = "another message from Alice to Bob!!";
        encryptedMsg1 = aliceSession.encryptMessage(clearMsg1);
        assertNotNull(encryptedMsg1);
        decryptedMsg1 = bobSession.decryptMessage(encryptedMsg1);
        assertNotNull(decryptedMsg1);
        assertTrue(clearMsg1.equals(decryptedMsg1));

        // comparison test
        assertTrue(clearMsg1.equals(decryptedMsg1));

        // clean objects..
        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));
        bobAccount.releaseAccount();
        aliceAccount.releaseAccount();
        assertTrue(0==bobAccount.getUnreleasedCount());
        assertTrue(0==aliceAccount.getUnreleasedCount());

        bobSession.releaseSession();
        aliceSession.releaseSession();
        assertTrue(0==bobSession.getUnreleasedCount());
        assertTrue(0==aliceSession.getUnreleasedCount());
    }


    @Test
    public void test03AliceBobSessionId() {
        // creates alice & bob accounts
        OlmAccount aliceAccount = null;
        OlmAccount bobAccount = null;
        try {
            aliceAccount = new OlmAccount();
            bobAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // CREATE ALICE SESSION

        OlmSession aliceSession = null;
        try {
            aliceSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE SESSION
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            e.printStackTrace();
        }
        assertTrue(0!=bobSession.getOlmSessionId());

        String aliceSessionId = aliceSession.sessionIdentifier();
        assertNotNull(aliceSessionId);

        String bobSessionId = bobSession.sessionIdentifier();
        assertNotNull(bobSessionId);

        // must be the same for both ends of the conversation
        assertTrue(aliceSessionId.equals(bobSessionId));

        aliceAccount.releaseAccount();
        bobAccount.releaseAccount();
        assertTrue(0==aliceAccount.getUnreleasedCount());
        assertTrue(0==bobAccount.getUnreleasedCount());

        bobSession.releaseSession();
        aliceSession.releaseSession();
        assertTrue(0==bobSession.getUnreleasedCount());
        assertTrue(0==aliceSession.getUnreleasedCount());
    }

    @Test
    public void test04MatchInboundSession() {
        OlmAccount aliceAccount=null, bobAccount=null;
        OlmSession aliceSession = null, bobSession = null;

        // ACCOUNTS CREATION
        try {
            aliceAccount = new OlmAccount();
            bobAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(), false);
        }

        // CREATE ALICE SESSION
        try {
            aliceSession = new OlmSession();
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg=" + e.getMessage(), false);
        }

        // get bob/luke identity key
        Map<String, String> bobIdentityKeys = bobAccount.identityKeys();
        Map<String, String> aliceIdentityKeys = aliceAccount.identityKeys();
        String bobIdentityKey = TestHelper.getIdentityKey(bobIdentityKeys);
        String aliceIdentityKey = TestHelper.getIdentityKey(aliceIdentityKeys);

        // get bob/luke one time keys
        assertTrue(0 == bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        assertTrue(0 == aliceAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        Map<String, Map<String, String>> bobOneTimeKeys = bobAccount.oneTimeKeys();
        String bobOneTimeKey1 = TestHelper.getOneTimeKey(bobOneTimeKeys, 1);

        // create alice inbound session for bob
        assertTrue(0==aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey1));

        String aliceClearMsg = "hello helooo to bob!";
        OlmMessage encryptedAliceToBobMsg1 = aliceSession.encryptMessage(aliceClearMsg);
        assertFalse(bobSession.matchesInboundSession(encryptedAliceToBobMsg1.mCipherText));

        // init bob session with alice PRE KEY
        assertTrue(0==bobSession.initInboundSessionWithAccount(bobAccount, encryptedAliceToBobMsg1.mCipherText));

        // test matchesInboundSession() and matchesInboundSessionFrom()
        assertTrue(bobSession.matchesInboundSession(encryptedAliceToBobMsg1.mCipherText));
        assertTrue(bobSession.matchesInboundSessionFrom(aliceIdentityKey, encryptedAliceToBobMsg1.mCipherText));
        // following requires olm native lib new version with https://github.com/matrix-org/olm-backup/commit/7e9f3bebb8390f975a76c0188ce4cb460fe6692e
        //assertTrue(false==bobSession.matchesInboundSessionFrom(bobIdentityKey, encryptedAliceToBobMsg1.mCipherText));

        // release objects
        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));
        aliceAccount.releaseAccount();
        bobAccount.releaseAccount();
        assertTrue(0==aliceAccount.getUnreleasedCount());
        assertTrue(0==bobAccount.getUnreleasedCount());

        aliceSession.releaseSession();
        bobSession.releaseSession();
        assertTrue(0==aliceSession.getUnreleasedCount());
        assertTrue(0==bobSession.getUnreleasedCount());
    }

    // ********************************************************
    // ************* SERIALIZATION TEST ***********************
    // ********************************************************
    /**
     * Same as {@link #test02AliceToBobBackAndForth()}, but alice's session
     * is serialized and de-serialized before performing the final
     * comparison (encrypt vs )
     */
    @Test
    public void test05SessionSerialization() {
        final int ONE_TIME_KEYS_NUMBER = 1;
        String bobIdentityKey;
        String bobOneTimeKey;
        OlmAccount aliceAccount = null;
        OlmAccount bobAccount = null;
        OlmSession aliceSessionDeserial = null;

        // creates alice & bob accounts
        try {
            aliceAccount = new OlmAccount();
            bobAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // get bob identity key
        Map<String, String> bobIdentityKeys = bobAccount.identityKeys();
        bobIdentityKey = TestHelper.getIdentityKey(bobIdentityKeys);
        assertTrue(null!=bobIdentityKey);

        // get bob one time keys
        assertTrue(0==bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        Map<String, Map<String, String>> bobOneTimeKeys = bobAccount.oneTimeKeys();
        bobOneTimeKey = TestHelper.getOneTimeKey(bobOneTimeKeys,1);
        assertNotNull(bobOneTimeKey);

        // CREATE ALICE SESSION
        OlmSession aliceSession = null;
        try {
            aliceSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE OUTBOUND SESSION and encrypt message to bob
        assertNotNull(aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));
        String helloClearMsg = "Hello I'm Alice!";

        OlmMessage encryptedAliceToBobMsg1 = aliceSession.encryptMessage(helloClearMsg);
        assertNotNull(encryptedAliceToBobMsg1);
        assertNotNull(encryptedAliceToBobMsg1.mCipherText);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
        assertTrue(0!=bobSession.getOlmSessionId());
        assertTrue(0==bobSession.initInboundSessionWithAccount(bobAccount, encryptedAliceToBobMsg1.mCipherText));

        // DECRYPT MESSAGE FROM ALICE
        String decryptedMsg01 = bobSession.decryptMessage(encryptedAliceToBobMsg1);
        assertNotNull(decryptedMsg01);

        // MESSAGE COMPARISON: decrypted vs encrypted
        assertTrue(helloClearMsg.equals(decryptedMsg01));

        // BACK/FORTH MESSAGE COMPARISON
        String clearMsg1 = "Hello I'm Bob!";
        String clearMsg2 = "Isn't life grand?";
        String clearMsg3 = "Let's go to the opera.";

        // bob encrypts messages
        OlmMessage encryptedMsg1 = bobSession.encryptMessage(clearMsg1);
        assertNotNull(encryptedMsg1);
        OlmMessage encryptedMsg2 = bobSession.encryptMessage(clearMsg2);
        assertNotNull(encryptedMsg2);
        OlmMessage encryptedMsg3 = bobSession.encryptMessage(clearMsg3);
        assertNotNull(encryptedMsg3);

        // serialize alice session
        Context context = getInstrumentation().getContext();
        try {
            FileOutputStream fileOutput = context.openFileOutput(FILE_NAME_SERIAL_SESSION, Context.MODE_PRIVATE);
            ObjectOutputStream objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(aliceSession);
            objectOutput.flush();
            objectOutput.close();

            // deserialize session
            FileInputStream fileInput = context.openFileInput(FILE_NAME_SERIAL_SESSION);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            aliceSessionDeserial = (OlmSession) objectInput.readObject();
            objectInput.close();

            // test deserialize return value
            assertNotNull(aliceSessionDeserial);

            // de-serialized alice session decrypts bob's messages
            String decryptedMsg1 = aliceSessionDeserial.decryptMessage(encryptedMsg1);
            assertNotNull(decryptedMsg1);
            String decryptedMsg2 = aliceSessionDeserial.decryptMessage(encryptedMsg2);
            assertNotNull(decryptedMsg2);
            String decryptedMsg3 = aliceSessionDeserial.decryptMessage(encryptedMsg3);
            assertNotNull(decryptedMsg3);

            // comparison tests
            assertTrue(clearMsg1.equals(decryptedMsg1));
            assertTrue(clearMsg2.equals(decryptedMsg2));
            assertTrue(clearMsg3.equals(decryptedMsg3));

            // clean objects..
            assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));
            bobAccount.releaseAccount();
            aliceAccount.releaseAccount();
            assertTrue(0==bobAccount.getUnreleasedCount());
            assertTrue(0==aliceAccount.getUnreleasedCount());

            bobSession.releaseSession();
            aliceSession.releaseSession();
            aliceSessionDeserial.releaseSession();
            assertTrue(0==bobSession.getUnreleasedCount());
            assertTrue(0==aliceSession.getUnreleasedCount());
            assertTrue(0==aliceSessionDeserial.getUnreleasedCount());
        }
        catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "## test03SessionSerialization(): Exception FileNotFoundException Msg=="+e.getMessage());
        }
        catch (ClassNotFoundException e) {
            Log.e(LOG_TAG, "## test03SessionSerialization(): Exception ClassNotFoundException Msg==" + e.getMessage());
        }
        catch (IOException e) {
            Log.e(LOG_TAG, "## test03SessionSerialization(): Exception IOException Msg==" + e.getMessage());
        }
        /*catch (OlmException e) {
            Log.e(LOG_TAG, "## test03SessionSerialization(): Exception OlmException Msg==" + e.getMessage());
        }*/
        catch (Exception e) {
            Log.e(LOG_TAG, "## test03SessionSerialization(): Exception Msg==" + e.getMessage());
        }
    }


    // ****************************************************
    // *************** SANITY CHECK TESTS *****************
    // ****************************************************

    @Test
    public void test06SanityCheckErrors() {
        final int ONE_TIME_KEYS_NUMBER = 5;
        OlmAccount bobAccount = null;
        OlmAccount aliceAccount = null;

        // ALICE & BOB ACCOUNTS CREATION
        try {
            aliceAccount = new OlmAccount();
            bobAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(), false);
        }

        // get bob identity key
        Map<String, String> bobIdentityKeys = bobAccount.identityKeys();
        String bobIdentityKey = TestHelper.getIdentityKey(bobIdentityKeys);
        assertTrue(null != bobIdentityKey);

        // get bob one time keys
        assertTrue(0 == bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        Map<String, Map<String, String>> bobOneTimeKeys = bobAccount.oneTimeKeys();
        assertNotNull(bobOneTimeKeys);
        String bobOneTimeKey = TestHelper.getOneTimeKey(bobOneTimeKeys,1);
        assertNotNull(bobOneTimeKey);

        // CREATE ALICE SESSION
        OlmSession aliceSession = null;
        try {
            aliceSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg=" + e.getMessage(), false);
        }

        // SANITY CHECK TESTS FOR: initOutboundSessionWithAccount()
        assertTrue(-1==aliceSession.initOutboundSessionWithAccount(null, bobIdentityKey, bobOneTimeKey));
        assertTrue(-1==aliceSession.initOutboundSessionWithAccount(aliceAccount, null, bobOneTimeKey));
        assertTrue(-1==aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, null));
        assertTrue(-1==aliceSession.initOutboundSessionWithAccount(null, null, null));

        // init properly
        assertTrue(0==aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));

        // SANITY CHECK TESTS FOR: encryptMessage()
        assertTrue(null==aliceSession.encryptMessage(null));

        // encrypt properly
        OlmMessage encryptedMsgToBob = aliceSession.encryptMessage("A message for bob");
        assertNotNull(encryptedMsgToBob);

        // SANITY CHECK TESTS FOR: initInboundSessionWithAccount()
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
            assertTrue(-1==bobSession.initInboundSessionWithAccount(null, encryptedMsgToBob.mCipherText));
            assertTrue(-1==bobSession.initInboundSessionWithAccount(bobAccount, null));
            assertTrue(-1==bobSession.initInboundSessionWithAccount(bobAccount, INVALID_PRE_KEY));
            // init properly
            assertTrue(0==bobSession.initInboundSessionWithAccount(bobAccount, encryptedMsgToBob.mCipherText));
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }

        // SANITY CHECK TESTS FOR: decryptMessage()
        String decryptedMsg = aliceSession.decryptMessage(null);
        assertTrue(null==decryptedMsg);

        // SANITY CHECK TESTS FOR: matchesInboundSession()
        assertTrue(!aliceSession.matchesInboundSession(null));

        // SANITY CHECK TESTS FOR: matchesInboundSessionFrom()
        assertTrue(!aliceSession.matchesInboundSessionFrom(null,null));

        // release objects
        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));
        aliceAccount.releaseAccount();
        bobAccount.releaseAccount();
        assertTrue(0==aliceAccount.getUnreleasedCount());
        assertTrue(0==bobAccount.getUnreleasedCount());

        aliceSession.releaseSession();
        bobSession.releaseSession();
        assertTrue(0==aliceSession.getUnreleasedCount());
        assertTrue(0==bobSession.getUnreleasedCount());
    }

}
