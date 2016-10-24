package org.matrix.olm;

import android.content.Context;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.json.JSONException;
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
import java.util.Iterator;

import static android.support.test.InstrumentationRegistry.getInstrumentation;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmSessionTest {
    private static final String LOG_TAG = "OlmSessionTest";
    private final String FILE_NAME_SERIAL_SESSION = "SerialSession";

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
            assertTrue("Exception Msg="+e.getMessage(), false);
        }

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
        Log.d(LOG_TAG,"## test01AliceToBob(): encryptedMsg="+encryptedMsgToBob.mCipherText);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
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
                // return first otk
                bobOneTimeKey = generatedKeys.getString(generatedKeysIt.next());
            }
            assertNotNull(bobOneTimeKey);
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

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

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
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

        // clean objects..
        bobAccount.releaseAccount();
        aliceAccount.releaseAccount();
        bobSession.releaseSession();
        aliceSession.releaseSession();
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
    }

    // ********************************************************
    // ************* SERIALIZATION TEST ***********************
    // ********************************************************
    /**
     * Same as test02AliceToBobBackAndForth() but alice's session
     * is serialized and de-serialized before performing the final
     * comparison (encrypt vs )
     */
    @Test
    public void test03SessionSerialization() {
        final int ONE_TIME_KEYS_NUMBER = 1;
        String bobIdentityKey = null;
        String bobOneTimeKey=null;
        OlmAccount aliceAccount = null;
        OlmAccount bobAccount = null;
        OlmSession aliceSessionDeserial;

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
                // return first otk
                bobOneTimeKey = generatedKeys.getString(generatedKeysIt.next());
            }
            assertNotNull(bobOneTimeKey);
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

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

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = null;
        try {
            bobSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
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
            bobAccount.releaseAccount();
            aliceAccount.releaseAccount();
            bobSession.releaseSession();
            aliceSession.releaseSession();
            aliceSessionDeserial.releaseSession();
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
}
