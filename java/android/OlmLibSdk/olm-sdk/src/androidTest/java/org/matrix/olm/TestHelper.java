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

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Iterator;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * Helper class providing helper methods used in the Olm Android SDK unit tests.
 */
public class TestHelper {

    /**
     * Return the identity key {@link OlmAccount#JSON_KEY_IDENTITY_KEY} from the JSON object.
     * @param aIdentityKeysObj JSON result of {@link OlmAccount#identityKeys()}
     * @return identity key string if operation succeed, null otherwise
     */
    static public String getIdentityKey(JSONObject aIdentityKeysObj){
        String idKey = null;

        try {
            idKey = aIdentityKeysObj.getString(OlmAccount.JSON_KEY_IDENTITY_KEY);
        } catch (JSONException e) {
            assertTrue("Exception MSg=" + e.getMessage(), false);
        }
        return idKey;
    }

    /**
     * Return the fingerprint key {@link OlmAccount#JSON_KEY_FINGER_PRINT_KEY} from the JSON object.
     * @param aIdentityKeysObj JSON result of {@link OlmAccount#identityKeys()}
     * @return fingerprint key string if operation succeed, null otherwise
     */
    static public String getFingerprintKey(JSONObject aIdentityKeysObj){
        String fingerprintKey = null;

        try {
            fingerprintKey = aIdentityKeysObj.getString(OlmAccount.JSON_KEY_FINGER_PRINT_KEY);
        } catch (JSONException e) {
            assertTrue("Exception MSg=" + e.getMessage(), false);
        }
        return fingerprintKey;
    }

    /**
     * Return the first one time key from the JSON object.
     * @param aIdentityKeysObj JSON result of {@link OlmAccount#oneTimeKeys()}
     * @param aKeyPosition the position of the key to be retrieved
     * @return one time key string if operation succeed, null otherwise
     */
    static public String getOneTimeKey(JSONObject aIdentityKeysObj, int aKeyPosition) {
        String firstOneTimeKey = null;
        int i=0;

        try {
            JSONObject generatedKeys = aIdentityKeysObj.getJSONObject(OlmAccount.JSON_KEY_ONE_TIME_KEY);
            assertNotNull(OlmAccount.JSON_KEY_ONE_TIME_KEY + " object is missing", generatedKeys);

            Iterator<String> generatedKeysIt = generatedKeys.keys();
            while(i<aKeyPosition) {
                if (generatedKeysIt.hasNext()) {
                    firstOneTimeKey = generatedKeys.getString(generatedKeysIt.next());
                    i++;
                }
            }
        } catch (JSONException e) {
            assertTrue("Exception Msg=" + e.getMessage(), false);
        }
        return firstOneTimeKey;
    }
}
