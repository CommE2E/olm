/**
 * Created by pedrocon on 13/10/2016.
 */
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


import android.text.TextUtils;
import android.util.Log;

import java.io.Serializable;

public class OlmInboundGroupSession implements Serializable {

    private static final String LOG_TAG = "OlmInboundGroupSession";

    /** session raw pointer value returned by JNI.<br>
     * this value uniquely identifies the native inbound group session instance.
     */
    private long mNativeOlmInboundGroupSessionId;


    public OlmInboundGroupSession() {
        initNewSession();
    }

    /**
     * Getter on the native inbound group session ID.
     * @return native inbound group session ID
     */
    public long getOlmInboundGroupSessionId(){
        return mNativeOlmInboundGroupSessionId;
    }

    /**
     * Release native session and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseSessionJni()}.
     * To be called before any other API call.
     */
    public void releaseSession(){
        releaseSessionJni();

        mNativeOlmInboundGroupSessionId = 0;
    }

    /**
     * Destroy the corresponding OLM inbound group session native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #initNewSessionJni()}.
     */
    private native void releaseSessionJni();

    /**
     * Create and save the session native instance ID.
     * Wrapper for {@link #initNewSessionJni()}.<br>
     * To be called before any other API call.
     * @return true if init succeed, false otherwise.
     */
    private boolean initNewSession() {
        boolean retCode = false;
        if(0 != (mNativeOlmInboundGroupSessionId = initNewSessionJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create the corresponding OLM inbound group session in native side.<br>
     * Do not forget to call {@link #releaseSession()} when JAVA side is done.
     * @return native session instance identifier (see {@link #mNativeOlmInboundGroupSessionId})
     */
    private native long initNewSessionJni();

    /**
     * Creates a new inbound group session.<br>
     * The session key parameter is retrieved from a outbound group session.
     * @param aSessionKey session key
     * @return 0 if operation succeed, -1 otherwise
     */
    public int initInboundGroupSessionWithSessionKey(String aSessionKey) {
        int retCode = -1;

        if(TextUtils.isEmpty(aSessionKey)){
            Log.e(LOG_TAG, "## initInboundGroupSessionWithSessionKey(): invalid session key");
        } else {
            retCode = initInboundGroupSessionWithSessionKeyJni(aSessionKey);
        }

        return retCode;
    }
    private native int initInboundGroupSessionWithSessionKeyJni(String aSessionKey);


    public String sessionIdentifier() {
        return sessionIdentifierJni();
    }
    private native String sessionIdentifierJni();


    public String decryptMessage(String aEncryptedMsg) {
        return decryptMessageJni(aEncryptedMsg);
    }
    private native String decryptMessageJni(String aEncryptedMsg);


    // TODO missing API: initWithSerializedData
    // TODO missing API: serializeDataWithKey
    // TODO missing API: initWithCoder
    // TODO missing API: encodeWithCoder
}
