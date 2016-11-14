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

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import static android.support.test.InstrumentationRegistry.getInstrumentation;
import static org.junit.Assert.assertFalse;
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
    private final String FILE_NAME = "SerialTestFile";

    @BeforeClass
    public static void setUpClass(){
        // enable UTF-8 specific conversion for pre Marshmallow(23) android versions,
        // due to issue described here: https://github.com/eclipsesource/J2V8/issues/142
        boolean isSpecificUtf8ConversionEnabled = android.os.Build.VERSION.SDK_INT < 23;

        // load native lib
        mOlmManager = new OlmManager(isSpecificUtf8ConversionEnabled);

        String olmLibVersion = mOlmManager.getOlmLibVersion();
        assertNotNull(olmLibVersion);
        String olmSdkVersion = mOlmManager.getSdkOlmVersion();
        assertNotNull(olmLibVersion);
        Log.d(LOG_TAG, "## setUpClass(): Versions - Android Olm SDK = "+olmSdkVersion+"  Olm lib ="+olmLibVersion);
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

    /**
     * Basic test: creation and release.
     */
    @Test
    public void test01CreateReleaseAccount() {
        try {
            mOlmAccount = new OlmAccount();
        } catch (OlmException e) {
            e.printStackTrace();
        }
        assertNotNull(mOlmAccount);

        mOlmAccount.releaseAccount();
        assertTrue(0 == mOlmAccount.getOlmAccountId());
    }

    @Test
    public void test02CreateAccount() {
        try {
            mOlmAccount = new OlmAccount();
        } catch (OlmException e) {
            e.printStackTrace();
        }
        assertNotNull(mOlmAccount);
        mIsAccountCreated = true;
    }

    @Test
    public void test04GetOlmAccountId() {
        long olmNativeInstance = mOlmAccount.getOlmAccountId();
        Log.d(LOG_TAG,"## testGetOlmAccountId olmNativeInstance="+olmNativeInstance);
        assertTrue(0!=olmNativeInstance);
    }

    /**
     * Test if {@link OlmAccount#identityKeys()} returns a JSON object
     * that contains the following keys: {@link OlmAccount#JSON_KEY_FINGER_PRINT_KEY}
     * and {@link OlmAccount#JSON_KEY_IDENTITY_KEY}
     */
    @Test
    public void test05IdentityKeys() {
        JSONObject identityKeysJson = mOlmAccount.identityKeys();
        assertNotNull(identityKeysJson);
        Log.d(LOG_TAG,"## testIdentityKeys Keys="+identityKeysJson);

        // is JSON_KEY_FINGER_PRINT_KEY present?
        String fingerPrintKey = TestHelper.getFingerprintKey(identityKeysJson);
        assertTrue("fingerprint key missing",!TextUtils.isEmpty(fingerPrintKey));

        // is JSON_KEY_IDENTITY_KEY present?
        String identityKey = TestHelper.getIdentityKey(identityKeysJson);
        assertTrue("identity key missing",!TextUtils.isEmpty(identityKey));
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

    /**
     * Test one time keys generation.
     */
    @Test
    public void test07GenerateOneTimeKeys() {
        int retValue = mOlmAccount.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        assertTrue(0==retValue);
    }

    /**
     * Test the generated amount of one time keys = GENERATION_ONE_TIME_KEYS_NUMBER.
     */
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
            oneTimeKeysCount = generatedKeysJsonObj.length();

            assertTrue("Expected count="+GENERATION_ONE_TIME_KEYS_NUMBER+" found="+oneTimeKeysCount,GENERATION_ONE_TIME_KEYS_NUMBER==oneTimeKeysCount);

        } catch (JSONException e) {
            assertTrue("Exception MSg="+e.getMessage(), false);
        }
    }

    @Test
    public void test10RemoveOneTimeKeysForSession() {
        OlmSession olmSession = null;
        try {
            olmSession = new OlmSession();
        } catch (OlmException e) {
            assertTrue("Exception Msg="+e.getMessage(), false);
        }
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
        FileOutputStream fileOutput;
        ObjectOutputStream objectOutput;
        OlmAccount accountRef = null;
        OlmAccount accountDeserial;

        try {
            accountRef = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        int retValue = accountRef.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        assertTrue(0==retValue);

        // get keys references
        JSONObject identityKeysRef = accountRef.identityKeys();
        JSONObject oneTimeKeysRef = accountRef.oneTimeKeys();
        assertNotNull(identityKeysRef);
        assertNotNull(oneTimeKeysRef);

        try {
            Context context = getInstrumentation().getContext();
            //context.getFilesDir();
            fileOutput = context.openFileOutput(FILE_NAME, Context.MODE_PRIVATE);

            // serialize account
            objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(accountRef);
            objectOutput.flush();
            objectOutput.close();

            // deserialize account
            FileInputStream fileInput = context.openFileInput(FILE_NAME);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            accountDeserial = (OlmAccount) objectInput.readObject();
            objectInput.close();
            assertNotNull(accountDeserial);

            // get de-serialized keys
            JSONObject identityKeysDeserial = accountDeserial.identityKeys();
            JSONObject oneTimeKeysDeserial = accountDeserial.oneTimeKeys();
            assertNotNull(identityKeysDeserial);
            assertNotNull(oneTimeKeysDeserial);

            // compare identity keys
            assertTrue(identityKeysDeserial.toString().equals(identityKeysRef.toString()));

            // compare onetime keys
            assertTrue(oneTimeKeysDeserial.toString().equals(oneTimeKeysRef.toString()));

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


    // ****************************************************
    // *************** SANITY CHECK TESTS *****************
    // ****************************************************

    @Test
    public void test14GenerateOneTimeKeysError() {
        // keys number = 0 => no error
        int retValue = mOlmAccount.generateOneTimeKeys(0);
        assertTrue(0==retValue);

        // keys number = negative value
        retValue = mOlmAccount.generateOneTimeKeys(-50);
        assertTrue(-1==retValue);
    }

    @Test
    public void test15RemoveOneTimeKeysForSessionError() {
        OlmAccount olmAccount = null;
        try {
            olmAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        int sessionRetCode = olmAccount.removeOneTimeKeysForSession(null);
        // test against no matching keys
        assertTrue(-1 == sessionRetCode);

        olmAccount.releaseAccount();
    }

    @Test
    public void test16SignMessageError() {
        OlmAccount olmAccount = null;
        try {
            olmAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }
        String signedMsg  = olmAccount.signMessage(null);
        assertNull(signedMsg);

        olmAccount.releaseAccount();
    }

    /**
     * Create multiple accounts and check that identity keys are still different.
     * This test validates random series are provide enough random values.
     */
    @Test
    public void test17MultipleAccountCreation() {
        try {
            OlmAccount account1 = new OlmAccount();
            OlmAccount account2 = new OlmAccount();
            OlmAccount account3 = new OlmAccount();
            OlmAccount account4 = new OlmAccount();
            OlmAccount account5 = new OlmAccount();
            OlmAccount account6 = new OlmAccount();
            OlmAccount account7 = new OlmAccount();
            OlmAccount account8 = new OlmAccount();
            OlmAccount account9 = new OlmAccount();
            OlmAccount account10 = new OlmAccount();

            JSONObject identityKeysJson1 = account1.identityKeys();
            JSONObject identityKeysJson2 = account2.identityKeys();
            JSONObject identityKeysJson3 = account3.identityKeys();
            JSONObject identityKeysJson4 = account4.identityKeys();
            JSONObject identityKeysJson5 = account5.identityKeys();
            JSONObject identityKeysJson6 = account6.identityKeys();
            JSONObject identityKeysJson7 = account7.identityKeys();
            JSONObject identityKeysJson8 = account8.identityKeys();
            JSONObject identityKeysJson9 = account9.identityKeys();
            JSONObject identityKeysJson10 = account10.identityKeys();

            String identityKey1 = TestHelper.getIdentityKey(identityKeysJson1);
            String identityKey2 = TestHelper.getIdentityKey(identityKeysJson2);
            assertFalse(identityKey1.equals(identityKey2));

            String identityKey3 = TestHelper.getIdentityKey(identityKeysJson3);
            assertFalse(identityKey2.equals(identityKey3));

            String identityKey4 = TestHelper.getIdentityKey(identityKeysJson4);
            assertFalse(identityKey3.equals(identityKey4));

            String identityKey5 = TestHelper.getIdentityKey(identityKeysJson5);
            assertFalse(identityKey4.equals(identityKey5));

            String identityKey6 = TestHelper.getIdentityKey(identityKeysJson6);
            assertFalse(identityKey5.equals(identityKey6));

            String identityKey7 = TestHelper.getIdentityKey(identityKeysJson7);
            assertFalse(identityKey6.equals(identityKey7));

            String identityKey8 = TestHelper.getIdentityKey(identityKeysJson8);
            assertFalse(identityKey7.equals(identityKey8));

            String identityKey9 = TestHelper.getIdentityKey(identityKeysJson9);
            assertFalse(identityKey8.equals(identityKey9));

            String identityKey10 = TestHelper.getIdentityKey(identityKeysJson10);
            assertFalse(identityKey9.equals(identityKey10));

            account1.releaseAccount();
            account2.releaseAccount();
            account3.releaseAccount();
            account4.releaseAccount();
            account5.releaseAccount();
            account6.releaseAccount();
            account7.releaseAccount();
            account8.releaseAccount();
            account9.releaseAccount();
            account10.releaseAccount();

        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }
    }
}
