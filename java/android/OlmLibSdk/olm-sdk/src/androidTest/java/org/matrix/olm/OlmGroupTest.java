package org.matrix.olm;

import android.support.test.runner.AndroidJUnit4;
import android.text.TextUtils;
import android.util.Log;

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;


import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmGroupTest {
    private static final String LOG_TAG = "OlmSessionTest";

    private static OlmManager mOlmManager;
    private static OlmOutboundGroupSession mAliceOutboundSession;
    private static String mAliceSessionIdentifier;
    private static long mAliceMessageIndex;
    public  static final String CLEAR_MESSAGE1 = "Hello!";
    private static String mAliceToBobMessage;
    private static OlmInboundGroupSession mBobInboundSession;
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
            mAliceOutboundSession = new OlmOutboundGroupSession();
        } catch (OlmException e) {
            assertTrue("Exception in OlmOutboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
    }

    @Test
    public void test02GetOutboundGroupSessionIdentifier() {
        // test session ID
        mAliceSessionIdentifier = mAliceOutboundSession.sessionIdentifier();
        assertNotNull(mAliceSessionIdentifier);
        assertTrue(mAliceSessionIdentifier.length() > 0);
    }

    @Test
    public void test03GetOutboundGroupSessionKey() {
        // test session Key
        mAliceOutboundSessionKey = mAliceOutboundSession.sessionKey();
        assertNotNull(mAliceOutboundSessionKey);
        assertTrue(mAliceOutboundSessionKey.length() > 0);
    }

    @Test
    public void test04GetOutboundGroupMessageIndex() {
        // test message index before any encryption
        mAliceMessageIndex = mAliceOutboundSession.messageIndex();
        assertTrue(0 == mAliceMessageIndex);
    }

    @Test
    public void test05OutboundGroupEncryptMessage() {
        // alice encrypts a message to bob
        mAliceToBobMessage = mAliceOutboundSession.encryptMessage(CLEAR_MESSAGE1);
        assertFalse(TextUtils.isEmpty(mAliceToBobMessage));

        // test message index after encryption is incremented
        mAliceMessageIndex = mAliceOutboundSession.messageIndex();
        assertTrue(1== mAliceMessageIndex);
    }

    @Test
    public void test06CreateInboundGroupSession() {
        // bob creates INBOUND GROUP SESSION with alice outbound key
        try {
            mBobInboundSession = new OlmInboundGroupSession(mAliceOutboundSessionKey);
        } catch (OlmException e) {
            assertTrue("Exception in bob OlmInboundGroupSession, Exception code=" + e.getExceptionCode(), false);
        }
    }

    @Test
    public void test07OutboundGroupSessionIdentifiers() {
        // check session identifiers are equals
        mAliceSessionIdentifier = mAliceOutboundSession.sessionIdentifier();
        assertFalse(TextUtils.isEmpty(mAliceSessionIdentifier));
    }

    @Test
    public void test08InboundGroupSessionIdentifiers() {
        // check session identifiers are equals
        mBobSessionIdentifier = mBobInboundSession.sessionIdentifier();
        assertFalse(TextUtils.isEmpty(mBobSessionIdentifier));
        assertTrue(mAliceSessionIdentifier.equals(mBobSessionIdentifier));
    }

    @Test
    public void test09SessionIdentifiersIdentical() {
        // check session identifiers are equals
        assertTrue(mAliceSessionIdentifier.equals(mBobSessionIdentifier));
    }

    @Test
    public void test10InboundDecryptMessage() {
        // test decrypted message
        mBobDecryptedMessage = mBobInboundSession.decryptMessage(mAliceToBobMessage);
        assertFalse(TextUtils.isEmpty(mBobDecryptedMessage));
        assertTrue(mBobDecryptedMessage.equals(CLEAR_MESSAGE1));
    }

    @Test
    public void test11InboundDecryptedMessageIdentical() {
        // test decrypted message
        assertTrue(mBobDecryptedMessage.equals(CLEAR_MESSAGE1));
    }

    @Test
    public void test12ReleaseOutboundSession() {
        // release group sessions
        mAliceOutboundSession.releaseSession();
    }

    @Test
    public void test13ReleaseInboundSession() {
        // release group sessions
        mBobInboundSession.releaseSession();
    }
}
