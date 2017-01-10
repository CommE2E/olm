/*
 * Copyright 2016 OpenMarket Ltd
 * Copyright 2016 Vector Creations Ltd
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
import java.util.Map;

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
        // load native lib
        mOlmManager = new OlmManager();

        String olmLibVersion = mOlmManager.getOlmLibVersion();
        assertNotNull(olmLibVersion);
        String olmSdkVersion = mOlmManager.getSdkOlmVersion(getInstrumentation().getContext());
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
            assertTrue("OlmAccount failed " + e.getMessage(), false);
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
            assertTrue("OlmAccount failed " + e.getMessage(), false);
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
        Map<String, String> identityKeys = null;
        try {
            identityKeys = mOlmAccount.identityKeys();
        } catch (Exception e) {
            assertTrue("identityKeys failed " + e.getMessage(), false);
        }
        assertNotNull(identityKeys);
        Log.d(LOG_TAG,"## testIdentityKeys Keys="+identityKeys);

        // is JSON_KEY_FINGER_PRINT_KEY present?
        String fingerPrintKey = TestHelper.getFingerprintKey(identityKeys);
        assertTrue("fingerprint key missing",!TextUtils.isEmpty(fingerPrintKey));

        // is JSON_KEY_IDENTITY_KEY present?
        String identityKey = TestHelper.getIdentityKey(identityKeys);
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
        String error = null;

        try {
            mOlmAccount.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        } catch (Exception e) {
            error = e.getMessage();
        }

        assertTrue(null == error);
    }

    /**
     * Test the generated amount of one time keys = GENERATION_ONE_TIME_KEYS_NUMBER.
     */
    @Test
    public void test08OneTimeKeysJsonFormat() {
        int oneTimeKeysCount = 0;
        Map<String, Map<String, String>> oneTimeKeysJson = null;

        try {
            oneTimeKeysJson = mOlmAccount.oneTimeKeys();
        } catch (Exception e) {
            assertTrue(e.getMessage(), false);
        }
        assertNotNull(oneTimeKeysJson);

        try {
            Map<String, String> map = oneTimeKeysJson.get(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertTrue(OlmAccount.JSON_KEY_ONE_TIME_KEY +" object is missing", null!=map);

            // test the count of the generated one time keys:
            oneTimeKeysCount = map.size();

            assertTrue("Expected count="+GENERATION_ONE_TIME_KEYS_NUMBER+" found="+oneTimeKeysCount,GENERATION_ONE_TIME_KEYS_NUMBER==oneTimeKeysCount);

        } catch (Exception e) {
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

        String errorMessage = null;

        try {
            mOlmAccount.removeOneTimeKeys(olmSession);
        } catch (Exception e) {
            errorMessage = e.getMessage();
        }
        assertTrue(null != errorMessage);

        olmSession.releaseSession();
        sessionId = olmSession.getOlmSessionId();
        assertTrue(0 == sessionId);
    }

    @Test
    public void test11MarkOneTimeKeysAsPublished() {
        try {
            mOlmAccount.markOneTimeKeysAsPublished();
        } catch (Exception e) {
            assertTrue(e.getMessage(), false);
        }
    }

    @Test
    public void test12SignMessage() {
        String clearMsg = "String to be signed by olm";
        String signedMsg  = null;

        try {
            signedMsg = mOlmAccount.signMessage(clearMsg);
        } catch (Exception e) {
            assertTrue(e.getMessage(), false);
        }

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
        OlmAccount accountDeserial = null;

        try {
            accountRef = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        try {
            accountRef.generateOneTimeKeys(GENERATION_ONE_TIME_KEYS_NUMBER);
        } catch (Exception e) {
            assertTrue(e.getMessage(),false);
        }

        // get keys references
        Map<String, String> identityKeysRef = null;

        try {
            identityKeysRef = accountRef.identityKeys();
        } catch (Exception e) {
            assertTrue("identityKeys failed " + e.getMessage(), false);
        }

        Map<String, Map<String, String>> oneTimeKeysRef = null;

        try {
            oneTimeKeysRef = accountRef.oneTimeKeys();
        } catch (Exception e) {
            assertTrue(e.getMessage(), false);
        }

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
            Map<String, String>  identityKeysDeserial = accountDeserial.identityKeys();
            Map<String, Map<String, String>> oneTimeKeysDeserial = accountDeserial.oneTimeKeys();
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
            assertTrue("test13Serialization failed " + e.getMessage(), false);
        }
        catch (ClassNotFoundException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception ClassNotFoundException Msg==" + e.getMessage());
            assertTrue("test13Serialization failed " + e.getMessage(), false);
        }
        catch (IOException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception IOException Msg==" + e.getMessage());
            assertTrue("test13Serialization failed " + e.getMessage(), false);
        }
        /*catch (OlmException e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception OlmException Msg==" + e.getMessage());
        }*/
        catch (Exception e) {
            Log.e(LOG_TAG, "## test13Serialization(): Exception Msg==" + e.getMessage());
            assertTrue("test13Serialization failed " + e.getMessage(), false);
        }
    }


    // ****************************************************
    // *************** SANITY CHECK TESTS *****************
    // ****************************************************

    @Test
    public void test14GenerateOneTimeKeysError() {
        // keys number = 0 => no error

        String errorMessage = null;
        try {
            mOlmAccount.generateOneTimeKeys(0);
        } catch (Exception e) {
            errorMessage = e.getMessage();
        }

        assertTrue(null == errorMessage);

        // keys number = negative value
        errorMessage = null;
        try {
            mOlmAccount.generateOneTimeKeys(-50);
        } catch (Exception e) {
            errorMessage = e.getMessage();
        }

        assertTrue(null != errorMessage);
    }

    @Test
    public void test15RemoveOneTimeKeysForSessionError() {
        OlmAccount olmAccount = null;
        try {
            olmAccount = new OlmAccount();
        } catch (OlmException e) {
            assertTrue(e.getMessage(),false);
        }

        try {
            olmAccount.removeOneTimeKeys(null);
        } catch (Exception e) {
            assertTrue(e.getMessage(), false);
        }

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
        String signedMsg = null;

        try {
            signedMsg = olmAccount.signMessage(null);
        } catch (Exception e) {
        }

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

            Map<String, String> identityKeys1 = account1.identityKeys();
            Map<String, String> identityKeys2 = account2.identityKeys();
            Map<String, String> identityKeys3 = account3.identityKeys();
            Map<String, String> identityKeys4 = account4.identityKeys();
            Map<String, String> identityKeys5 = account5.identityKeys();
            Map<String, String> identityKeys6 = account6.identityKeys();
            Map<String, String> identityKeys7 = account7.identityKeys();
            Map<String, String> identityKeys8 = account8.identityKeys();
            Map<String, String> identityKeys9 = account9.identityKeys();
            Map<String, String> identityKeys10 = account10.identityKeys();

            String identityKey1 = TestHelper.getIdentityKey(identityKeys1);
            String identityKey2 = TestHelper.getIdentityKey(identityKeys2);
            assertFalse(identityKey1.equals(identityKey2));

            String identityKey3 = TestHelper.getIdentityKey(identityKeys3);
            assertFalse(identityKey2.equals(identityKey3));

            String identityKey4 = TestHelper.getIdentityKey(identityKeys4);
            assertFalse(identityKey3.equals(identityKey4));

            String identityKey5 = TestHelper.getIdentityKey(identityKeys5);
            assertFalse(identityKey4.equals(identityKey5));

            String identityKey6 = TestHelper.getIdentityKey(identityKeys6);
            assertFalse(identityKey5.equals(identityKey6));

            String identityKey7 = TestHelper.getIdentityKey(identityKeys7);
            assertFalse(identityKey6.equals(identityKey7));

            String identityKey8 = TestHelper.getIdentityKey(identityKeys8);
            assertFalse(identityKey7.equals(identityKey8));

            String identityKey9 = TestHelper.getIdentityKey(identityKeys9);
            assertFalse(identityKey8.equals(identityKey9));

            String identityKey10 = TestHelper.getIdentityKey(identityKeys10);
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
