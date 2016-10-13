package org.matrix.olm;

import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;


import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmGroupTest {
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
    public void test00AliceToBob() {
        // TBD
    }

    /**
     * Basic test:
     * - alice creates an account
     * - bob creates an account
     * - alice creates an outbound group session
     * - bob creates an inbound group session with alice's outbound session key
     * - alice encrypts a message with its session
     * - bob decrypts the encrypted message with its session
     */
    //@Test
    public void test01AliceToBob() {
        // creates alice outbound session
        OlmOutboundGroupSession aliceOutboundSession = new OlmOutboundGroupSession();

        // test accounts creation
        String aliceSessionIdentifier = aliceOutboundSession.sessionIdentifier();
        assertNotNull(aliceSessionIdentifier);
        assertTrue(aliceSessionIdentifier.length()>0);

        String aliceOutboundSessionKey = aliceOutboundSession.sessionKey();
        assertNotNull(aliceOutboundSessionKey);
        assertTrue(aliceOutboundSessionKey.length()>0);

        long messageIndex = aliceOutboundSession.messageIndex();
        assertTrue(0==messageIndex);

        String clearMessage = "Hello!";
        String encryptedMessage = aliceOutboundSession.encryptMessage(clearMessage);
        assertNotNull(encryptedMessage);

        messageIndex = aliceOutboundSession.messageIndex();
        assertTrue(1==messageIndex);

        assertTrue(encryptedMessage.length()>=0);

        OlmInboundGroupSession bobInboundSession = new OlmInboundGroupSession();
        bobInboundSession.initInboundGroupSessionWithSessionKey(aliceOutboundSessionKey);
        // check session identifiers are equals
        aliceSessionIdentifier = aliceOutboundSession.sessionIdentifier();
        String bobSessionIdentifier = aliceOutboundSession.sessionIdentifier();
        assertTrue(aliceSessionIdentifier.equals(bobSessionIdentifier ));

        String decryptedMessage = bobInboundSession.decryptMessage(encryptedMessage);
        assertTrue(decryptedMessage.equals(bobSessionIdentifier ));
    }

    //@Test
    public void test02InboundGroupSession() {
        // creates alice outbound session
        OlmInboundGroupSession aliceInboundSession = new OlmInboundGroupSession();

        // test session identifier
        String sessionIdentifier = aliceInboundSession.sessionIdentifier();
        assertNotNull(sessionIdentifier);
        assertTrue(sessionIdentifier.length()>0);
    }

}
