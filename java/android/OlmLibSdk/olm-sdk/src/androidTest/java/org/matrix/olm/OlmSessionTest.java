package org.matrix.olm;

import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import java.util.Iterator;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmSessionTest {
    private static final String LOG_TAG = "OlmSessionTest";

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

        // creates alice & bob accounts
        OlmAccount aliceAccount = new OlmAccount();
        OlmAccount bobAccount = new OlmAccount();

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // get bob identity key
        JSONObject bobIdentityKeysJson = bobAccount.identityKeys();
        assertNotNull(bobIdentityKeysJson);
        try {
            bobIdentityKey = bobIdentityKeysJson.getString(OlmAccount.JSON_KEY_IDENTITY_KEY);
            assertTrue(null!=bobIdentityKey);
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

        // get bob one time keys
        assertTrue(0==bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        JSONObject bobOneTimeKeysJsonObj = bobAccount.oneTimeKeys();
        assertNotNull(bobOneTimeKeysJsonObj);
        try {
            JSONObject generatedKeys = bobOneTimeKeysJsonObj.getJSONObject(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertNotNull(OlmAccount.JSON_KEY_ONE_TIME_KEY +" object is missing", generatedKeys);

            Iterator<String> generatedKeysIt = generatedKeys.keys();
            if(generatedKeysIt.hasNext()) {
                bobOneTimeKey = generatedKeys.getString(generatedKeysIt.next());
            }
            assertNotNull(bobOneTimeKey);
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

        // CREATE ALICE SESSION
        OlmSession aliceSession = new OlmSession();
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE OUTBOUND SESSION and encrypt message to bob
        assertNotNull(aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));
        String clearMsg = "Heloo bob , this is alice!";
        OlmMessage encryptedMsgToBob = aliceSession.encryptMessage(clearMsg);
        assertNotNull(encryptedMsgToBob);
        Log.d(LOG_TAG,"## test01AliceToBob(): encryptedMsg="+encryptedMsgToBob.mCipherText);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = new OlmSession();
        assertTrue(0!=bobSession.getOlmSessionId());
        assertNotNull(bobSession.initInboundSessionWithAccount(bobAccount, encryptedMsgToBob.mCipherText));
        String decryptedMsg = bobSession.decryptMessage(encryptedMsgToBob);
        assertNotNull(decryptedMsg);

        // MESSAGE COMPARISON: decrypted vs encrypted
        assertTrue(clearMsg.equals(decryptedMsg));

        // clean objects..
        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));
        // release accounts
        bobAccount.releaseAccount();
        aliceAccount.releaseAccount();
        // release sessions
        bobSession.releaseSession();
        aliceSession.releaseSession();
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
     */
    @Test
    public void test02AliceToBobBackAndForth() {
        final int ONE_TIME_KEYS_NUMBER = 1;
        String bobIdentityKey = null;
        String bobOneTimeKey=null;

        // creates alice & bob accounts
        OlmAccount aliceAccount = new OlmAccount();
        OlmAccount bobAccount = new OlmAccount();

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // get bob identity key
        JSONObject bobIdentityKeysJson = bobAccount.identityKeys();
        assertNotNull(bobIdentityKeysJson);
        try {
            bobIdentityKey = bobIdentityKeysJson.getString(OlmAccount.JSON_KEY_IDENTITY_KEY);
            assertTrue(null!=bobIdentityKey);
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

        // get bob one time keys
        assertTrue(0==bobAccount.generateOneTimeKeys(ONE_TIME_KEYS_NUMBER));
        JSONObject bobOneTimeKeysJsonObj = bobAccount.oneTimeKeys();
        assertNotNull(bobOneTimeKeysJsonObj);
        try {
            JSONObject generatedKeys = bobOneTimeKeysJsonObj.getJSONObject(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertNotNull(OlmAccount.JSON_KEY_ONE_TIME_KEY +" object is missing", generatedKeys);

            Iterator<String> generatedKeysIt = generatedKeys.keys();
            if(generatedKeysIt.hasNext()) {
                bobOneTimeKey = generatedKeys.getString(generatedKeysIt.next());
            }
            assertNotNull(bobOneTimeKey);
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

        // CREATE ALICE SESSION
        OlmSession aliceSession = new OlmSession();
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE OUTBOUND SESSION and encrypt message to bob
        assertNotNull(aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));
        String helloClearMsg = "Hello I'm Alice!";

        OlmMessage encryptedAliceToBobMsg1 = aliceSession.encryptMessage(helloClearMsg);
        assertNotNull(encryptedAliceToBobMsg1);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = new OlmSession();
        assertTrue(0!=bobSession.getOlmSessionId());
        assertNotNull(bobSession.initInboundSessionWithAccount(bobAccount, encryptedAliceToBobMsg1.mCipherText));

        // DECRYPT MESSAGE FROM ALICE
        String decryptedMsg01 = bobSession.decryptMessage(encryptedAliceToBobMsg1);
        assertNotNull(decryptedMsg01);

        // MESSAGE COMPARISON: decrypted vs encrypted
        assertTrue(helloClearMsg.equals(decryptedMsg01));

        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession));

        // BACK/FORTH MESSAGE COMPARISON
        String clearMsg1 = "Hello I'm Bob!";
        String clearMsg2 = "Isn't life grand?";
        String clearMsg3 = "Let's go to the opera.";

        OlmMessage encryptedMsg1 = bobSession.encryptMessage(clearMsg1);
        assertNotNull(encryptedMsg1);
        OlmMessage encryptedMsg2 = bobSession.encryptMessage(clearMsg2);
        assertNotNull(encryptedMsg2);
        OlmMessage encryptedMsg3 = bobSession.encryptMessage(clearMsg3);
        assertNotNull(encryptedMsg3);

        String decryptedMsg1 = aliceSession.decryptMessage(encryptedMsg1);
        assertNotNull(decryptedMsg1);
        String decryptedMsg2 = aliceSession.decryptMessage(encryptedMsg2);
        assertNotNull(decryptedMsg2);
        String decryptedMsg3 = aliceSession.decryptMessage(encryptedMsg3);
        assertNotNull(decryptedMsg3);

        assertTrue(clearMsg1.equals(decryptedMsg1));
        assertTrue(clearMsg2.equals(decryptedMsg2));
        assertTrue(clearMsg3.equals(decryptedMsg3));

        // clean objects..
        bobAccount.releaseAccount();
        aliceAccount.releaseAccount();
        bobSession.releaseSession();
        aliceSession.releaseSession();
    }

    @Test
    public void test03AliceBobSessionId() {
        // creates alice & bob accounts
        OlmAccount aliceAccount = new OlmAccount();
        OlmAccount bobAccount = new OlmAccount();

        // test accounts creation
        assertTrue(0!=bobAccount.getOlmAccountId());
        assertTrue(0!=aliceAccount.getOlmAccountId());

        // CREATE ALICE SESSION
        OlmSession aliceSession = new OlmSession();
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = new OlmSession();
        assertTrue(0!=bobSession.getOlmSessionId());

        String aliceSessionId = aliceSession.sessionIdentifier();
        assertNotNull(aliceSessionId);

        String bobSessionId = bobSession.sessionIdentifier();
        assertNotNull(bobSessionId);

        // must be the same for both ends of the conversation
        assertTrue(aliceSessionId.equals(bobSessionId));
    }

}
