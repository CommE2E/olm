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

    @Test
    public void test01AliceToBob() {
        String bobIdentityKey = null;
        String bobOneTimeKey=null;

        // creates alice & bob accounts
        OlmAccount aliceAccount = new OlmAccount();
        aliceAccount.initNewAccount();

        OlmAccount bobAccount = new OlmAccount();
        bobAccount.initNewAccount();

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
        assertTrue(0==bobAccount.generateOneTimeKeys(5));
        JSONObject bobOneTimeKeysJsonObj = bobAccount.oneTimeKeys();
        assertNotNull(bobOneTimeKeysJsonObj);
        try {
            JSONObject generatedKeys = bobOneTimeKeysJsonObj.getJSONObject(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertNotNull(OlmAccount.JSON_KEY_ONE_TIME_KEY +" object is missing", generatedKeys);

            // test the count of the generated one time keys:
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
        aliceSession.initNewSession();
        assertTrue(0!=aliceSession.getOlmSessionId());

        // CREATE ALICE OUTBOUND SESSION and encrypt message to bob
        assertNotNull(aliceSession.initOutboundSessionWithAccount(aliceAccount, bobIdentityKey, bobOneTimeKey));
        String clearMsg = "Heloo bob , this is alice!";
        OlmMessage encryptedMsgToBob = aliceSession.encryptMessage(clearMsg);
        assertNotNull(encryptedMsgToBob);
        Log.d(LOG_TAG,"## test01AliceToBob(): encryptedMsg="+encryptedMsgToBob.mCipherText);

        // CREATE BOB INBOUND SESSION and decrypt message from alice
        OlmSession bobSession = new OlmSession();
        bobSession.initNewSession();
        assertTrue(0!=bobSession.getOlmSessionId());
        assertNotNull(bobSession.initInboundSessionWithAccount(bobAccount, encryptedMsgToBob.mCipherText));
        String decryptedMsg = bobSession.decryptMessage(encryptedMsgToBob);
        assertNotNull(decryptedMsg);

        // MESSAGE COMPARISON: decrypted vs encrypted
        assertTrue(clearMsg.equals(decryptedMsg));

        // clean objects..
        assertTrue(0==bobAccount.removeOneTimeKeysForSession(bobSession.getOlmSessionId()));
        bobAccount.releaseAccount();
        aliceAccount.releaseAccount();
        bobSession.releaseSession();
        aliceSession.releaseSession();
    }
}
