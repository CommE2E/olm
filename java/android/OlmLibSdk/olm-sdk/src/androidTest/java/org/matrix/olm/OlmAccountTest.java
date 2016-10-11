package org.matrix.olm;

import android.support.test.runner.AndroidJUnit4;
import android.text.TextUtils;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
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
public class OlmAccountTest {
    private static final String LOG_TAG = "OlmAccountTest";
    private static final int GENERATION_ONE_TIME_KEYS_NUMBER = 50;

    private static OlmAccount mOlmAccount;
    private static OlmManager mOlmManager;
    private boolean mIsAccountCreated;

    @BeforeClass
    public static void setUpClass(){
        // load native lib
        mOlmManager = new OlmManager();

        String version = mOlmManager.getOlmLibVersion();
        assertNotNull(version);
        Log.d(LOG_TAG, "## setUpClass(): lib version="+version);
    }

    @AfterClass
    public static void tearDownClass() {
        // TBD
    }

    @Before
    public void setUp() {
        if(mIsAccountCreated) {
            assertNotNull(mOlmAccount);
        }
    }

    @After
    public void tearDown() {
        // TBD
    }

    @Test
    public void test01CreateReleaseAccount() {
        mOlmAccount = new OlmAccount();
        assertNotNull(mOlmAccount);

        mOlmAccount.releaseAccount();
        assertTrue(0 == mOlmAccount.getOlmAccountId());
    }

    @Test
    public void test02CreateAccount() {
        mOlmAccount = new OlmAccount();
        assertNotNull(mOlmAccount);
    }

    @Test
    public void test03InitNewAccount() {
        assertTrue(mOlmAccount.initNewAccount());
        mIsAccountCreated = true;
    }

    @Test
    public void test04GetOlmAccountId() {
        long olmNativeInstance = mOlmAccount.getOlmAccountId();
        Log.d(LOG_TAG,"## testGetOlmAccountId olmNativeInstance="+olmNativeInstance);
        assertTrue(0!=olmNativeInstance);
    }

    @Test
    public void test05IdentityKeys() {
        JSONObject identityKeysJson = mOlmAccount.identityKeys();
        assertNotNull(identityKeysJson);
        Log.d(LOG_TAG,"## testIdentityKeys Keys="+identityKeysJson);

        try {
            String fingerPrintKey = identityKeysJson.getString(OlmAccount.JSON_KEY_FINGER_PRINT_KEY);
            assertTrue("fingerprint key missing",!TextUtils.isEmpty(fingerPrintKey));
        } catch (JSONException e) {
            e.printStackTrace();
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

        try {
            String identityKey = identityKeysJson.getString(OlmAccount.JSON_KEY_IDENTITY_KEY);
            assertTrue("identity key missing",!TextUtils.isEmpty(identityKey));
        } catch (JSONException e) {
            e.printStackTrace();
            assertTrue("Exception MSg="+e.getMessage(), false);
        }


    }

    //****************************************************
    //** ************** One time keys TESTS **************
    //****************************************************
    @Test
    public void test06MaxOneTimeKeys() {
        long maxOneTimeKeys = mOlmAccount.maxOneTimeKeys();
        Log.d(LOG_TAG,"## testMaxOneTimeKeys(): maxOneTimeKeys="+maxOneTimeKeys);

        assertTrue(maxOneTimeKeys>0);
    }

    @Test
    public void test07GenerateOneTimeKeys() {
        int retValue = mOlmAccount.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        assertTrue(0==retValue);
    }

    @Test
    public void test08OneTimeKeysJsonFormat() {
        int oneTimeKeysCount = 0;
        JSONObject generatedKeysJsonObj;
        JSONObject oneTimeKeysJson = mOlmAccount.oneTimeKeys();
        assertNotNull(oneTimeKeysJson);

        try {
            generatedKeysJsonObj = oneTimeKeysJson.getJSONObject(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertTrue(OlmAccount.JSON_KEY_ONE_TIME_KEY +" object is missing", null!=generatedKeysJsonObj);

            // test the count of the generated one time keys:
            Iterator<String> generatedKeysIt = generatedKeysJsonObj.keys();
            while(generatedKeysIt.hasNext()){
                generatedKeysIt.next();
                oneTimeKeysCount++;
            }
            assertTrue("Expected count="+GENERATION_ONE_TIME_KEYS_NUMBER+" found="+oneTimeKeysCount,GENERATION_ONE_TIME_KEYS_NUMBER==oneTimeKeysCount);

        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }
    }

    @Test
    public void test10RemoveOneTimeKeysForSession() {
        OlmSession olmSession = new OlmSession();
        olmSession.initNewSession();
        long sessionId = olmSession.getOlmSessionId();
        assertTrue(0 != sessionId);

        int sessionRetCode = mOlmAccount.removeOneTimeKeysForSession(sessionId);
        // no one time key has been use in the session, so removeOneTimeKeysForSession() returns an error
        assertTrue(0 != sessionRetCode);

        olmSession.releaseSession();
        sessionId = olmSession.getOlmSessionId();
        assertTrue("sessionRetCode="+sessionRetCode,0 == sessionId);
    }

    @Test
    public void test11MarkOneTimeKeysAsPublished() {
        int retCode = mOlmAccount.markOneTimeKeysAsPublished();
        // if OK => retCode=0
        assertTrue(0 == retCode);
    }

    @Test
    public void test12SignMessage() {
        String clearMsg = "String to be signed by olm";
        String signedMsg  = mOlmAccount.signMessage(clearMsg);
        assertNotNull(signedMsg);
        // TODO add test to unsign the signedMsg and compare it ot clearMsg
    }


    private void testJni(){
        OlmManager mgr = new OlmManager();
        String versionLib = mgr.getOlmLibVersion();
        Log.d(LOG_TAG, "## testJni(): lib version="+versionLib);

        OlmAccount account = new OlmAccount();
        boolean initStatus = account.initNewAccount();

        long accountNativeId = account.getOlmAccountId();
        Log.d(LOG_TAG, "## testJni(): lib accountNativeId="+accountNativeId);

        JSONObject identityKeys = account.identityKeys();
        Log.d(LOG_TAG, "## testJni(): identityKeysJson="+identityKeys.toString());

        long maxOneTimeKeys = account.maxOneTimeKeys();
        Log.d(LOG_TAG, "## testJni(): lib maxOneTimeKeys="+maxOneTimeKeys);

        int generateRetCode = account.generateOneTimeKeys(50);
        Log.d(LOG_TAG, "## testJni(): generateRetCode="+generateRetCode);

        JSONObject oneTimeKeysKeysJson = account.oneTimeKeys();
        Log.d(LOG_TAG, "## testJni(): oneTimeKeysKeysJson="+oneTimeKeysKeysJson.toString());

        int asPublishedRetCode = account.markOneTimeKeysAsPublished();
        Log.d(LOG_TAG, "## testJni(): asPublishedRetCode="+asPublishedRetCode);

        String clearMsg ="My clear message";
        String signedMsg = account.signMessage(clearMsg);
        Log.d(LOG_TAG, "## testJni(): signedMsg="+signedMsg);

        account.releaseAccount();
    }


}
