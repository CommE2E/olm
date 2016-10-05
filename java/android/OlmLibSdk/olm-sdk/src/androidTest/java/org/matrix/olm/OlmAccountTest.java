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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmAccountTest {
    private static final String LOG_TAG = "OlmAccountTest";
    private static final int GENERATION_ONE_TIME_KEYS_NUMBER = 50;

    private static OlmAccount mOlmAccount;
    private static OlmManager mOlmManager;
    private boolean mIsAccountCreated;

    public static final String TEST_STRING = "This is a string";
    public static final long TEST_LONG = 12345678L;

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
    public void test1CreateAccount() {
        Log.d(LOG_TAG,"## testInitNewAccount");
        mOlmAccount = new OlmAccount();
        assertNotNull(mOlmAccount);
    }

    @Test
    public void test2InitNewAccount() {
        Log.d(LOG_TAG,"## testInitNewAccount");
        assertTrue(mOlmAccount.initNewAccount());
        mIsAccountCreated = true;
    }

    @Test
    public void test3GetOlmAccountId() {
        Log.d(LOG_TAG,"## testGetOlmAccountId");

        long olmNativeInstance = mOlmAccount.getOlmAccountId();
        assertTrue(0!=olmNativeInstance);
    }

    @Test
    public void test4IdentityKeys() {
        Log.d(LOG_TAG,"## testIdentityKeys");

        JSONObject identityKeysJson = mOlmAccount.identityKeys();
        assertNotNull(identityKeysJson);
        Log.d(LOG_TAG,"## testIdentityKeys Keys="+identityKeysJson);

        try {
            String fingerPrintKey = identityKeysJson.getString(OlmAccount.JSON_KEY_FINGER_PRINT_KEY);
            assertFalse("fingerprint key missing",TextUtils.isEmpty(fingerPrintKey));
        } catch (JSONException e) {
            e.printStackTrace();
            assertTrue("Exception MSg="+e.getMessage(), false);
        }

        try {
            String identityKey = identityKeysJson.getString(OlmAccount.JSON_KEY_IDENTITY_KEY);
            assertFalse("identity key missing",TextUtils.isEmpty(identityKey));
        } catch (JSONException e) {
            e.printStackTrace();
            assertTrue("Exception MSg="+e.getMessage(), false);
        }


    }


    //****************************************************
    //** ************** One time keys TESTS **************
    //****************************************************
    @Test
    public void test5MaxOneTimeKeys() {
        Log.d(LOG_TAG,"## testMaxOneTimeKeys");

        long maxOneTimeKeys = mOlmAccount.maxOneTimeKeys();
        Log.d(LOG_TAG,"## testMaxOneTimeKeys(): maxOneTimeKeys="+maxOneTimeKeys);

        assertTrue(maxOneTimeKeys>0);
    }

    @Test
    public void test6GenerateOneTimeKeys() {
        Log.d(LOG_TAG,"## testGenerateOneTimeKeys");
        int retValue = mOlmAccount.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        assertTrue(0==retValue);
    }

    @Test
    public void test7OneTimeKeysJsonFormat() {
        Log.d(LOG_TAG,"## testIdentityKeys");
        JSONObject generatedKeysJsonObj;
        JSONObject oneTimeKeysJson = mOlmAccount.oneTimeKeys();
        assertNotNull(oneTimeKeysJson);

        try {
            generatedKeysJsonObj = oneTimeKeysJson.getJSONObject(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertFalse(OlmAccount.JSON_KEY_ONE_TIME_KEY +" object is missing", null==generatedKeysJsonObj);

            /*String oneTimeKeyA = generatedKeysJsonObj.getString(OlmAccount.JSON_KEY_ONE_TIME_KEY_GENERATED_A);
            assertFalse(" one time KeyA object is missing", TextUtils.isEmpty(oneTimeKeyA));

            String oneTimeKeyB = generatedKeysJsonObj.getString(OlmAccount.JSON_KEY_ONE_TIME_KEY_GENERATED_B);
            assertFalse(" one time KeyA object is missing", TextUtils.isEmpty(oneTimeKeyA));*/
        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }
    }

    // TODO testRemoveOneTimeKeysForSession when session is available
    /*@Test
    public void testRemoveOneTimeKeysForSession() {
        Log.d(LOG_TAG,"## testRemoveOneTimeKeysForSession");
        OLMSession olmSession = new OLMSession();

        JSONArray keysJsonArray = mOlmAccount.removeOneTimeKeysForSession(olmSession);

        assertNotNull(keysJsonArray);
        // TODO add extra test to test the JSON content format..
    }*/

    @Test
    public void test8MarkOneTimeKeysAsPublished() {
        Log.d(LOG_TAG,"## testMarkOneTimeKeysAsPublished");

        int retCode = mOlmAccount.markOneTimeKeysAsPublished();
        // if OK => retCode=0
        assertTrue(0 == retCode);
    }

    @Test
    public void test9SignMessage() {
        Log.d(LOG_TAG,"## testMarkOneTimeKeysAsPublished");

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

        JSONObject onteTimeKeysKeysJson = account.oneTimeKeys();
        Log.d(LOG_TAG, "## testJni(): onteTimeKeysKeysJson="+onteTimeKeysKeysJson.toString());

        // TODO removeOneTimeKeysForSession(session);

        int asPublishedRetCode = account.markOneTimeKeysAsPublished();
        Log.d(LOG_TAG, "## testJni(): asPublishedRetCode="+asPublishedRetCode);

        String clearMsg ="My clear message";
        String signedMsg = account.signMessage(clearMsg);
        Log.d(LOG_TAG, "## testJni(): signedMsg="+signedMsg);

        account.releaseAccount();
    }


}
