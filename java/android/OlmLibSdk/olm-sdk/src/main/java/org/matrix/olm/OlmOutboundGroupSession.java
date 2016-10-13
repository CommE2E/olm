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

public class OlmOutboundGroupSession {
    private static final String LOG_TAG = "OlmOutboundGroupSession";

    /** session raw pointer value returned by JNI.<br>
     * this value uniquely identifies the native inbound group session instance.
     */
    private long mNativeOlmOutboundGroupSessionId;

    public OlmOutboundGroupSession() {
        initNewSession();
    }

    /**
     * Getter on the native outbound group session ID.
     * @return native outbound group session ID
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

        mNativeOlmOutboundGroupSessionId = 0;
    }

    /**
     * Destroy the corresponding OLM outbound group session native object.<br>
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
        if(0 != (mNativeOlmOutboundGroupSessionId = initNewSessionJni())){
            retCode = true;
        }
        return retCode;
    }

    /**
     * Create the corresponding OLM outbound group session in native side.<br>
     * Do not forget to call {@link #releaseSession()} when JAVA side is done.
     * @return native session instance identifier (see {@link #mNativeOlmOutboundGroupSessionId})
     */
    private native long initNewSessionJni();


    /**
     * Creates a new outbound group session.<br>
     * The session key parameter is retrieved from a outbound group session.
     * @return 0 if operation succeed, -1 otherwise
     */
    public int initOutboundGroupSession() {
        return initOutboundGroupSessionJni();
    }
    public native int initOutboundGroupSessionJni();




    public String sessionIdentifier() {
        String retValue = null;
        //retValue = sessionIdentifierJni();

        return retValue;
    }
    public native String sessionIdentifierJni();




    public long messageIndex() {
        long retValue =0;
        //retValue = messageIndexJni();

        return retValue;
    }
    private native long messageIndexJni();




    public String sessionKey() {
        String retValue = null;
        //retValue = sessionKeyJni();

        return retValue;
    }
    private native String sessionKeyJni();


    public String encryptMessage(String aClearMsg) {
        String retValue = null;
        //retValue = encryptMessageJni(aClearMsg);

        return retValue;
    }
    private native String encryptMessageJni(String aClearMsg);
}
