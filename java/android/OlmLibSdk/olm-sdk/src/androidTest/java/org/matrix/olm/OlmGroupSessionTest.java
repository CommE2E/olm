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
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmGroupSessionTest {
    private static final String LOG_TAG = "OlmSessionTest";
    private final String FILE_NAME_SERIAL_OUT_SESSION = "SerialOutGroupSession";
    private final String FILE_NAME_SERIAL_IN_SESSION = "SerialInGroupSession";

    private static OlmManager mOlmManager;
    private static OlmOutboundGroupSession mAliceOutboundGroupSession;
    private static String mAliceSessionIdentifier;
    private static long mAliceMessageIndex;
    private static final String CLEAR_MESSAGE1 = "Hello!";
    private static String mAliceToBobMessage;
    private static OlmInboundGroupSession mBobInboundGroupSession;
    private static String mAliceOutboundSessionKey;
    private static String mBobSessionIdentifier;
    private static String mBobDecryptedMessage;

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
     * - alice creates an outbound group session
     * - bob creates an inbound group session with alice's outbound session key
     * - alice encrypts a message with its session
     * - bob decrypts the encrypted message with its session
     * - decrypted message is identical to original alice message
     */
    @Test
    public void test01CreateOutboundSession() {
        // alice creates OUTBOUND GROUP SESSION
        try {
            mAliceOutboundGroupSession = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            assertTrue("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
    }

    @Test
    public void test02GetOutboundGroupSessionIdentifier() {
        // test session ID
        mAliceSessionIdentifier = mAliceOutboundGroupSession.sessionIdentifier();
        assertNotNull(mAliceSessionIdentifier);
        assertTrue(mAliceSessionIdentifier.length() > 0);
    }

    @Test
    public void test03GetOutboundGroupSessionKey() {
        // test session Key
        mAliceOutboundSessionKey = mAliceOutboundGroupSession.sessionKey();
        assertNotNull(mAliceOutboundSessionKey);
        assertTrue(mAliceOutboundSessionKey.length() > 0);
    }

    @Test
    public void test04GetOutboundGroupMessageIndex() {
        // test message index before any encryption
        mAliceMessageIndex = mAliceOutboundGroupSession.messageIndex();
        assertTrue(0 == mAliceMessageIndex);
    }

    @Test
    public void test05OutboundGroupEncryptMessage() {
        // alice encrypts a message to bob
        mAliceToBobMessage = mAliceOutboundGroupSession.encryptMessage(CLEAR_MESSAGE1);
        assertFalse(TextUtils.isEmpty(mAliceToBobMessage));

        // test message index after encryption is incremented
        mAliceMessageIndex = mAliceOutboundGroupSession.messageIndex();
        assertTrue(1 == mAliceMessageIndex);
    }

    @Test
    public void test06CreateInboundGroupSession() {
        // bob creates INBOUND GROUP SESSION with alice outbound key
        try {
            mBobInboundGroupSession = new OlmInboundGroupSession(mAliceOutboundSessionKey);
        } catch (OlmException e) {
            assertTrue("Exception in bob OlmInboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
    }

    @Test
    public void test08GetInboundGroupSessionIdentifier() {
        // check both session identifiers are equals
        mBobSessionIdentifier = mBobInboundGroupSession.sessionIdentifier();
        assertFalse(TextUtils.isEmpty(mBobSessionIdentifier));
    }

    @Test
    public void test09SessionIdentifiersAreIdentical() {
        // check both session identifiers are equals: alice vs bob
        assertTrue(mAliceSessionIdentifier.equals(mBobSessionIdentifier));
    }

    @Test
    public void test10InboundDecryptMessage() {
        // test decrypted message
        mBobDecryptedMessage = mBobInboundGroupSession.decryptMessage(mAliceToBobMessage);
        assertFalse(TextUtils.isEmpty(mBobDecryptedMessage));
    }

    @Test
    public void test11InboundDecryptedMessageIdentical() {
        // test decrypted message
        assertTrue(mBobDecryptedMessage.equals(CLEAR_MESSAGE1));
    }

    @Test
    public void test12ReleaseOutboundSession() {
        // release group sessions
        mAliceOutboundGroupSession.releaseSession();
    }

    @Test
    public void test13ReleaseInboundSession() {
        // release group sessions
        mBobInboundGroupSession.releaseSession();
    }


    @Test
    public void test14SerializeOutboundSession() {
        OlmOutboundGroupSession outboundGroupSessionRef=null;
        OlmOutboundGroupSession outboundGroupSessionSerial;

        // create one OUTBOUND GROUP SESSION
        try {
            outboundGroupSessionRef = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            assertTrue("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
        assertNotNull(outboundGroupSessionRef);


        // serialize alice session
        Context context = getInstrumentation().getContext();
        try {
            FileOutputStream fileOutput = context.openFileOutput(FILE_NAME_SERIAL_OUT_SESSION, Context.MODE_PRIVATE);
            ObjectOutputStream objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(outboundGroupSessionRef);
            objectOutput.flush();
            objectOutput.close();

            // deserialize session
            FileInputStream fileInput = context.openFileInput(FILE_NAME_SERIAL_OUT_SESSION);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            outboundGroupSessionSerial = (OlmOutboundGroupSession) objectInput.readObject();
            objectInput.close();

            // get sessions keys
            String sessionKeyRef = outboundGroupSessionRef.sessionKey();
            String sessionKeySerial = outboundGroupSessionSerial.sessionKey();

            // session keys sanity check
            assertFalse(TextUtils.isEmpty(sessionKeyRef));
            assertFalse(TextUtils.isEmpty(sessionKeySerial));

            // session keys comparison
            assertTrue(sessionKeyRef.equals(sessionKeySerial));

            // get sessions IDs
            String sessionIdRef = outboundGroupSessionRef.sessionIdentifier();
            String sessionIdSerial = outboundGroupSessionSerial.sessionIdentifier();

            // session ID sanity check
            assertFalse(TextUtils.isEmpty(sessionIdRef));
            assertFalse(TextUtils.isEmpty(sessionIdSerial));

            // session IDs comparison
            assertTrue(sessionIdRef.equals(sessionIdSerial));

            outboundGroupSessionRef.releaseSession();
            outboundGroupSessionSerial.releaseSession();
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

    @Test
    public void test15SerializeInboundSession() {
        OlmOutboundGroupSession aliceOutboundGroupSession=null;
        OlmInboundGroupSession bobInboundGroupSessionRef=null;
        OlmInboundGroupSession bobInboundGroupSessionSerial;

        // alice creates OUTBOUND GROUP SESSION
        try {
            aliceOutboundGroupSession = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            assertTrue("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
        assertNotNull(aliceOutboundGroupSession);

        // get the session key from the outbound group session
        String sessionKeyRef = aliceOutboundGroupSession.sessionKey();
        assertNotNull(sessionKeyRef);

        // bob creates INBOUND GROUP SESSION
        try {
            bobInboundGroupSessionRef = new OlmInboundGroupSession(sessionKeyRef);
        } catch (OlmException e) {
            assertTrue("Exception in OlmInboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
        assertNotNull(bobInboundGroupSessionRef);


        // serialize alice session
        Context context = getInstrumentation().getContext();
        try {
            FileOutputStream fileOutput = context.openFileOutput(FILE_NAME_SERIAL_IN_SESSION, Context.MODE_PRIVATE);
            ObjectOutputStream objectOutput = new ObjectOutputStream(fileOutput);
            objectOutput.writeObject(bobInboundGroupSessionRef);
            objectOutput.flush();
            objectOutput.close();

            // deserialize session
            FileInputStream fileInput = context.openFileInput(FILE_NAME_SERIAL_OUT_SESSION);
            ObjectInputStream objectInput = new ObjectInputStream(fileInput);
            bobInboundGroupSessionSerial = (OlmInboundGroupSession)objectInput.readObject();
            objectInput.close();

            // get sessions IDs
            String aliceSessionId = aliceOutboundGroupSession.sessionIdentifier();
            String sessionIdRef = bobInboundGroupSessionRef.sessionIdentifier();
            String sessionIdSerial = bobInboundGroupSessionSerial.sessionIdentifier();

            // session ID sanity check
            assertFalse(TextUtils.isEmpty(aliceSessionId));
            assertFalse(TextUtils.isEmpty(sessionIdRef));
            assertFalse(TextUtils.isEmpty(sessionIdSerial));

            // session IDs comparison
            assertTrue(aliceSessionId.equals(sessionIdSerial));
            assertTrue(sessionIdRef.equals(sessionIdSerial));
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
        catch (Exception e) {
            Log.e(LOG_TAG, "## test03SessionSerialization(): Exception Msg==" + e.getMessage());
        }
    }

    /**
     * Create multiple outbound group sessions and check that session Keys are different.
     * This test validates random series are provide enough random values.
     */
    @Test
    public void test16MultipleOutboundSession() {
        OlmOutboundGroupSession outboundGroupSession1;
        OlmOutboundGroupSession outboundGroupSession2;
        OlmOutboundGroupSession outboundGroupSession3;
        OlmOutboundGroupSession outboundGroupSession4;
        OlmOutboundGroupSession outboundGroupSession5;
        OlmOutboundGroupSession outboundGroupSession6;
        OlmOutboundGroupSession outboundGroupSession7;
        OlmOutboundGroupSession outboundGroupSession8;

        try {
            outboundGroupSession1 = new OlmOutboundGroupSession();
            outboundGroupSession2 = new OlmOutboundGroupSession();
            outboundGroupSession3 = new OlmOutboundGroupSession();
            outboundGroupSession4 = new OlmOutboundGroupSession();
            outboundGroupSession5 = new OlmOutboundGroupSession();
            outboundGroupSession6 = new OlmOutboundGroupSession();
            outboundGroupSession7 = new OlmOutboundGroupSession();
            outboundGroupSession8 = new OlmOutboundGroupSession();

            // get the session key from the outbound group sessions
            String sessionKey1 = outboundGroupSession1.sessionKey();
            String sessionKey2 = outboundGroupSession2.sessionKey();
            assertFalse(sessionKey1.equals(sessionKey2));

            String sessionKey3 = outboundGroupSession3.sessionKey();
            assertFalse(sessionKey2.equals(sessionKey3));

            String sessionKey4 = outboundGroupSession4.sessionKey();
            assertFalse(sessionKey3.equals(sessionKey4));

            String sessionKey5 = outboundGroupSession5.sessionKey();
            assertFalse(sessionKey4.equals(sessionKey5));

            String sessionKey6 = outboundGroupSession6.sessionKey();
            assertFalse(sessionKey5.equals(sessionKey6));

            String sessionKey7 = outboundGroupSession7.sessionKey();
            assertFalse(sessionKey6.equals(sessionKey7));

            String sessionKey8 = outboundGroupSession8.sessionKey();
            assertFalse(sessionKey7.equals(sessionKey8));

            // get the session IDs from the outbound group sessions
            String sessionId1 = outboundGroupSession1.sessionIdentifier();
            String sessionId2 = outboundGroupSession2.sessionIdentifier();
            assertFalse(sessionId1.equals(sessionId2));

            String sessionId3 = outboundGroupSession3.sessionKey();
            assertFalse(sessionId2.equals(sessionId3));

            String sessionId4 = outboundGroupSession4.sessionKey();
            assertFalse(sessionId3.equals(sessionId4));

            String sessionId5 = outboundGroupSession5.sessionKey();
            assertFalse(sessionId4.equals(sessionId5));

            String sessionId6 = outboundGroupSession6.sessionKey();
            assertFalse(sessionId5.equals(sessionId6));

            String sessionId7 = outboundGroupSession7.sessionKey();
            assertFalse(sessionId6.equals(sessionId7));

            String sessionId8 = outboundGroupSession8.sessionKey();
            assertFalse(sessionId7.equals(sessionId8));


        } catch (OlmException e) {
            assertTrue("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
    }

}
