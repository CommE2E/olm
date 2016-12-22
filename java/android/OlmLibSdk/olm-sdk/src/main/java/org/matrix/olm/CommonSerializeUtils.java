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

import android.text.TextUtils;
import android.util.Log;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

/**
 * Helper class dedicated to serialization mechanism (template method pattern).
 */
abstract class CommonSerializeUtils {
    private static final String LOG_TAG = "CommonSerializeUtils";

    /**
     * Kick off the serialization mechanism.
     * @param aOutStream output stream for serializing
     * @throws IOException exception
     */
    protected void serializeObject(ObjectOutputStream aOutStream) throws IOException {
        aOutStream.defaultWriteObject();

        // generate serialization key
        String key = OlmUtility.getRandomKey();

        // compute pickle string
        StringBuffer errorMsg = new StringBuffer();
        String pickledData = serializeDataWithKey(key, errorMsg);

        if(null == pickledData) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_SERIALIZATION, String.valueOf(errorMsg));
        } else {
            aOutStream.writeObject(key);
            aOutStream.writeObject(pickledData);
        }
    }

    /**
     * Kick off the deserialization mechanism.
     * @param aInStream input stream
     * @throws IOException exception
     * @throws ClassNotFoundException exception
     */
    protected void deserializeObject(ObjectInputStream aInStream) throws IOException, ClassNotFoundException {
        aInStream.defaultReadObject();
        StringBuffer errorMsg = new StringBuffer();

        String key = (String) aInStream.readObject();
        String pickledData = (String) aInStream.readObject();

        if(TextUtils.isEmpty(key)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" key");

        } else if(TextUtils.isEmpty(pickledData)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, OlmException.EXCEPTION_MSG_INVALID_PARAMS_DESERIALIZATION+" pickle");

        } else if(!createNewObjectFromSerialization()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, OlmException.EXCEPTION_MSG_INIT_NEW_ACCOUNT_DESERIALIZATION);

        } else if(!initWithSerializedData(pickledData, key, errorMsg)) {
            releaseObjectFromSerialization(); // prevent memory leak
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, String.valueOf(errorMsg));

        } else {
            Log.d(LOG_TAG,"## readObject(): success");
        }
    }

    protected abstract String serializeDataWithKey(String aKey, StringBuffer aErrorMsg);
    protected abstract boolean initWithSerializedData(String aSerializedData, String aKey, StringBuffer aErrorMsg);
    protected abstract boolean createNewObjectFromSerialization();
    protected abstract void releaseObjectFromSerialization();
}
