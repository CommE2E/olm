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

import android.util.Log;

/**
 * Olm SDK entry point class.<br> An OlmManager instance must be created at first to enable native library load.
 * <br><br>Detailed implementation guide is available at <a href="http://matrix.org/docs/guides/e2e_implementation.html">Implementing End-to-End Encryption in Matrix clients</a>.
 */
public class OlmManager {
    private static final String LOG_TAG = "OlmManager";
    private static final String SDK_OLM_VERSION = "V0.1.0_1";

    /**
     * Constructor.
     */
    public OlmManager() {
    }

    static {
        try {
            java.lang.System.loadLibrary("olm");
        } catch(UnsatisfiedLinkError e) {
            Log.e(LOG_TAG,"Exception loadLibrary() - Msg="+e.getMessage());
        }
    }

    public String getSdkOlmVersion() {
        return SDK_OLM_VERSION;
    }

    /**
     * Get the OLM lib version.
     * @return the lib version as a string
     */
    public String getOlmLibVersion(){
        return getOlmLibVersionJni();
    }
    public native String getOlmLibVersionJni();
}

