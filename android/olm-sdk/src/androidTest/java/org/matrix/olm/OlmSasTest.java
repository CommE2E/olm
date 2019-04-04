/*
 * Copyright 2019 New Vector Ltd
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


import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class OlmSasTest {

    private static OlmManager mOlmManager;

    //Enable the native lib
    @BeforeClass
    public static void setUpClass() {
        // load native librandomBytesOfLength
        mOlmManager = new OlmManager();
    }

    @Test
    public void testSASCode() {
        OlmSAS aliceSas = null;
        OlmSAS bobSas = null;

        try {
            aliceSas = new OlmSAS();
            bobSas = new OlmSAS();

            String alicePKey = aliceSas.getPublicKey();
            String bobPKey = bobSas.getPublicKey();

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice pub Key is " + alicePKey);
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob pub Key is " + bobPKey);

            aliceSas.setTheirPublicKey(bobPKey);
            bobSas.setTheirPublicKey(alicePKey);

            int codeLength = 6;
            byte[] alice_sas = aliceSas.generateShortCode("SAS", codeLength);
            byte[] bob_sas = bobSas.generateShortCode("SAS", codeLength);

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice SAS is " + new String(alice_sas, "UTF-8"));
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob SAS is " + new String(bob_sas, "UTF-8"));

            assertEquals(codeLength, alice_sas.length);
            assertEquals(codeLength, bob_sas.length);
            assertArrayEquals(alice_sas, bob_sas);

            byte[] aliceMac = aliceSas.calculateMac("Hello world!", "SAS");
            byte[] bobMac = bobSas.calculateMac("Hello world!", "SAS");

            assertTrue(aliceMac.length > 0 && bobMac.length > 0);
            assertEquals(aliceMac.length, bobMac.length);
            assertArrayEquals(aliceMac, bobMac);

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice Mac is " + new String(aliceMac, "UTF-8"));
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob Mac is " + new String(bobMac, "UTF-8"));


            byte[] aliceLongKdfMac = aliceSas.calculateMacLongKdf("Hello world!", "SAS");
            byte[] bobLongKdfMac = bobSas.calculateMacLongKdf("Hello world!", "SAS");

            assertTrue(aliceLongKdfMac.length > 0 && bobLongKdfMac.length > 0);
            assertEquals(aliceLongKdfMac.length, bobLongKdfMac.length);
            assertArrayEquals(aliceLongKdfMac, bobLongKdfMac);

            Log.e(OlmSasTest.class.getSimpleName(), "#### Alice lkdf Mac is " + new String(aliceLongKdfMac, "UTF-8"));
            Log.e(OlmSasTest.class.getSimpleName(), "#### Bob lkdf Mac is " + new String(bobLongKdfMac, "UTF-8"));


        } catch (Exception e) {
            assertTrue("OlmSas init failed " + e.getMessage(), false);
            e.printStackTrace();
        } finally {
            if (aliceSas != null) {
                aliceSas.releaseSas();
            }
            if (bobSas != null) {
                bobSas.releaseSas();
            }
        }
    }

}
