/*
Copyright 2018 New Vector Ltd

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

"use strict";

var Olm = require('../olm');

if (!Object.keys) {
    Object.keys = function(o) {
        var k=[], p;
        for (p in o) if (Object.prototype.hasOwnProperty.call(o,p)) k.push(p);
        return k;
    }
}

describe("pk", function() {
    var encryption, decryption;

    beforeEach(function() {
        encryption = new Olm.PkEncryption();
        decryption = new Olm.PkDecryption();
    });

    afterEach(function () {
        if (encryption !== undefined) {
            encryption.free();
            encryption = undefined;
        }
        if (decryption !== undefined) {
            decryption.free();
            decryption = undefined;
        }
    });

    it('should encrypt and decrypt', function () {
        var TEST_TEXT='têst1';
        var pubkey = decryption.generate_key();
        encryption.set_recipient_key(pubkey);
        var encrypted = encryption.encrypt(TEST_TEXT);
        var decrypted = decryption.decrypt(encrypted.ephemeral, encrypted.mac, encrypted.ciphertext);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        TEST_TEXT='hot beverage: ☕';
        encryption.set_recipient_key(pubkey);
        encrypted = encryption.encrypt(TEST_TEXT);
        decrypted = decryption.decrypt(encrypted.ephemeral, encrypted.mac, encrypted.ciphertext);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);
    });
});
