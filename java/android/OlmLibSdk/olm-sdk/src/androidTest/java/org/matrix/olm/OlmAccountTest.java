package org.matrix.olm;

import android.accounts.Account;
import android.content.Context;
import android.content.SharedPreferences;
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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Iterator;

import static android.support.test.InstrumentationRegistry.getInstrumentation;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
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
    //***************** ONE TIME KEYS TESTS **************
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
        long sessionId = olmSession.getOlmSessionId();
        assertTrue(0 != sessionId);

        int sessionRetCode = mOlmAccount.removeOneTimeKeysForSession(olmSession);
        // test against no matching keys
        assertTrue(1 == sessionRetCode);

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
        // additional tests are performed in test01VerifyEd25519Signing()
    }


    // ********************************************************
    // ************* SERIALIZATION TEST ***********************
    // ********************************************************

    @Test
    public void test13Serialization() {
        FileOutputStream fileOutput = null;
        ObjectOutputStream objectOutput = null;
        OlmAccount accountRef = new OlmAccount();
        OlmAccount accountDeserial = null;
        OlmException exception;

        int retValue = accountRef.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        assertTrue(0==retValue);

        JSONObject identityKeysRef = accountRef.identityKeys();
        JSONObject oneTimeKeysRef = accountRef.oneTimeKeys();
        final String FILE_NAME = "testfile";

        /*Context context = getInstrumentation().getContext();
        SharedPreferences sharedPref = context.getSharedPreferences("TestPref",Context.MODE_PRIVATE);
        SharedPreferences.Editor editPref = sharedPref.edit();
        editPref.putLong();*/

        try {
            Context context = getInstrumentation().getContext();
            context.getFilesDir();
            //File serialFile = new File(FILE_NAME);
            //fileOutput = new FileOutputStream(serialFile);
            fileOutput = context.openFileOutput(FILE_NAME, Context.MODE_PRIVATE);

            objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(accountRef);
            objectOutput.flush();
            objectOutput.close();

            //FileInputStream fileInput = new FileInputStream(serialFile);
            FileInputStream fileInput = context.openFileInput(FILE_NAME);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            accountDeserial = (OlmAccount) objectInput.readObject();
            objectInput.close();

            assertNotNull(accountDeserial);

            JSONObject identityKeys2 = accountDeserial.identityKeys();
            JSONObject oneTimeKeys2 = accountDeserial.oneTimeKeys();
            assertEquals(identityKeysRef, identityKeys2);
            assertEquals(oneTimeKeysRef, oneTimeKeys2);

            accountRef.releaseAccount();
            accountDeserial.releaseAccount();

        }

        catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception FileNotFoundException Msg=="+e.getMessage());
        }
        catch (ClassNotFoundException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception ClassNotFoundException Msg==" + e.getMessage());
        }
        catch (IOException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception IOException Msg==" + e.getMessage());
        }
        /*catch (OlmException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception OlmException Msg==" + e.getMessage());
        }*/
        catch (Exception e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception Msg==" + e.getMessage());
        }

    }


}
