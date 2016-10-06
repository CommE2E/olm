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

public class OlmMessage {
    /** PRE KEY message type (used to establish new Olm session) **/
    public final static int MESSAGE_TYPE_PRE_KEY = 0;
    /** normal message type **/
    public final static int MESSAGE_TYPE_MESSAGE = 1;

    /** the encrypted message (ie. )**/
    public String mCipherText;

    /** defined by {@link #MESSAGE_TYPE_MESSAGE} or {@link #MESSAGE_TYPE_PRE_KEY}**/
    public long mType;
}
