/*
Copyright 2016 OpenMarket Ltd
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

describe("olm", function() {
    var aliceAccount, bobAccount;
    var aliceSession, bobSession;

    beforeEach(function(done) {
        // This should really be in a beforeAll, but jasmine-node
        // doesn't support that
        Olm.init().then(function() {
            aliceAccount = new Olm.Account();
            bobAccount = new Olm.Account();
            aliceSession = new Olm.Session();
            bobSession = new Olm.Session();

            aliceAccount.create();
            bobAccount.create();

            bobAccount.generate_prekey();
            bobAccount.mark_prekey_as_published();
            bobAccount.generate_prekey();
            bobAccount.mark_prekey_as_published();
            bobAccount.forget_old_prekey();

            done();
        });
    });

    afterEach(function() {
        if (aliceAccount !== undefined) {
            aliceAccount.free();
            aliceAccount = undefined;
        }

        if (bobAccount !== undefined) {
            bobAccount.free();
            bobAccount = undefined;
        }

        if (aliceSession !== undefined) {
            aliceSession.free();
            aliceSession = undefined;
        }

        if (bobSession !== undefined) {
            bobSession.free();
            bobSession = undefined;
        }
    });

    function testPickleAndRestore(){
        var alicePickleKey = 'SomeSecretAlice';
        var bobPickleKey = 'SomeSecretBob';
        var aliceSessionPickled = aliceSession.pickle(alicePickleKey);
        var bobSessionPickled = bobSession.pickle(bobPickleKey);

        aliceSession = new Olm.Session();
        bobSession = new Olm.Session();

        aliceSession.unpickle(alicePickleKey, aliceSessionPickled);
        bobSession.unpickle(bobPickleKey, bobSessionPickled);
    }

    function testEncryptDecrypt() {
        var TEST_TEXT='têst1';
        var encrypted = aliceSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(0);
        bobSession.create_inbound(bobAccount, encrypted.body);
        bobAccount.remove_one_time_keys(bobSession);
        var decrypted = bobSession.decrypt(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        TEST_TEXT='hot beverage: ☕';
        encrypted = bobSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(1);
        decrypted = aliceSession.decrypt(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        testPickleAndRestore();

        TEST_TEXT='some emoji: ☕ 123 // after pickling';
        encrypted = bobSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(1);
        decrypted = aliceSession.decrypt(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);
    }

    function testEncryptDecryptSequential() {
        var TEST_TEXT='têst1';
        var encrypted = aliceSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(0);
        bobSession.create_inbound(bobAccount, encrypted.body);
        bobAccount.remove_one_time_keys(bobSession);
        var decrypted = bobSession.decrypt_sequential(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        TEST_TEXT='hot beverage: ☕';
        encrypted = bobSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(1);
        decrypted = aliceSession.decrypt_sequential(encrypted.type, encrypted.body);
        console.log(TEST_TEXT, "->", decrypted);
        expect(decrypted).toEqual(TEST_TEXT);

        testPickleAndRestore();

        var TEST_TEXT_1 ='some emoji: ☕ 123';
        var encrypted_1 = bobSession.encrypt(TEST_TEXT_1);
        expect(encrypted_1.type).toEqual(1);

        var TEST_TEXT_2 ='some emoji: ☕ 456 ';
        var encrypted_2 = bobSession.encrypt(TEST_TEXT_2);
        expect(encrypted_2.type).toEqual(1);

        expect(
            () => aliceSession.decrypt_sequential(encrypted_2.type, encrypted_2.body)
        ).toThrow();

        var decrypted1 = aliceSession.decrypt(encrypted_1.type, encrypted_1.body);
        console.log(TEST_TEXT_1, "->", decrypted1);
        expect(decrypted1).toEqual(TEST_TEXT_1);

        var decrypted2 = aliceSession.decrypt(encrypted_2.type, encrypted_2.body);
        console.log(TEST_TEXT_2, "->", decrypted2);
        expect(decrypted2).toEqual(TEST_TEXT_2);
    }

    it('should encrypt and decrypt with session created with OTK', function() {
        bobAccount.generate_one_time_keys(1);
        var bobOneTimeKeys = JSON.parse(bobAccount.one_time_keys()).curve25519;
        bobAccount.mark_keys_as_published();

        var bobIdKey = JSON.parse(bobAccount.identity_keys()).curve25519;
        var bobSigningKey = JSON.parse(bobAccount.identity_keys()).ed25519;

        var bobPrekey = Object.values(JSON.parse(bobAccount.prekey()).curve25519)[0];
        var bobPreKeySignature = bobAccount.prekey_signature();

        var otk_id = Object.keys(bobOneTimeKeys)[0];

        aliceSession.create_outbound(
            aliceAccount, bobIdKey, bobSigningKey, bobPrekey, bobPreKeySignature, bobOneTimeKeys[otk_id]
        );

        testEncryptDecrypt();
    });

    it('should encrypt and decrypt sequential with session created with OTK', function() {
        bobAccount.generate_one_time_keys(1);
        var bobOneTimeKeys = JSON.parse(bobAccount.one_time_keys()).curve25519;
        bobAccount.mark_keys_as_published();

        var bobIdKey = JSON.parse(bobAccount.identity_keys()).curve25519;
        var bobSigningKey = JSON.parse(bobAccount.identity_keys()).ed25519;

        var bobPrekey = Object.values(JSON.parse(bobAccount.prekey()).curve25519)[0];
        var bobPreKeySignature = bobAccount.prekey_signature();

        var otk_id = Object.keys(bobOneTimeKeys)[0];

        aliceSession.create_outbound(
            aliceAccount, bobIdKey, bobSigningKey, bobPrekey, bobPreKeySignature, bobOneTimeKeys[otk_id]
        );

        testEncryptDecryptSequential();
    });

    it('should encrypt and decrypt with session created without OTK', function() {
        var bobIdKey = JSON.parse(bobAccount.identity_keys()).curve25519;
        var bobSigningKey = JSON.parse(bobAccount.identity_keys()).ed25519;

        var bobPrekey = Object.values(JSON.parse(bobAccount.prekey()).curve25519)[0];
        var bobPreKeySignature = bobAccount.prekey_signature();

        aliceSession.create_outbound_without_otk(
            aliceAccount, bobIdKey, bobSigningKey, bobPrekey, bobPreKeySignature
        );

        testEncryptDecrypt();
    });

    it('should encrypt and decrypt sequential with session created without OTK', function() {
        var bobIdKey = JSON.parse(bobAccount.identity_keys()).curve25519;
        var bobSigningKey = JSON.parse(bobAccount.identity_keys()).ed25519;

        var bobPrekey = Object.values(JSON.parse(bobAccount.prekey()).curve25519)[0];
        var bobPreKeySignature = bobAccount.prekey_signature();

        aliceSession.create_outbound_without_otk(
            aliceAccount, bobIdKey, bobSigningKey, bobPrekey, bobPreKeySignature
        );

        testEncryptDecryptSequential();
    });

    it('should handle sender chain initialization and received_message flag setting', function() {
        bobAccount.generate_one_time_keys(1);
        var bobOneTimeKeys = JSON.parse(bobAccount.one_time_keys()).curve25519;
        bobAccount.mark_keys_as_published();

        var bobIdKey = JSON.parse(bobAccount.identity_keys()).curve25519;
        var bobSigningKey = JSON.parse(bobAccount.identity_keys()).ed25519;

        var bobPrekey = Object.values(JSON.parse(bobAccount.prekey()).curve25519)[0];
        var bobPreKeySignature = bobAccount.prekey_signature();

        var otk_id = Object.keys(bobOneTimeKeys)[0];

        expect(aliceSession.is_sender_chain_empty()).toEqual(true);
        aliceSession.create_outbound(
            aliceAccount, bobIdKey, bobSigningKey, bobPrekey, bobPreKeySignature, bobOneTimeKeys[otk_id]
        );
        expect(aliceSession.is_sender_chain_empty()).toEqual(false);
        expect(aliceSession.has_received_message()).toEqual(false);

        var TEST_TEXT='têst1';
        var encrypted = aliceSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(0);
        bobSession.create_inbound(bobAccount, encrypted.body);
        bobAccount.remove_one_time_keys(bobSession);
        var decrypted = bobSession.decrypt(encrypted.type, encrypted.body);
        expect(decrypted).toEqual(TEST_TEXT);

        expect(bobSession.is_sender_chain_empty()).toEqual(true);
        expect(bobSession.has_received_message()).toEqual(true);

        TEST_TEXT='hot beverage: ☕';
        encrypted = bobSession.encrypt(TEST_TEXT);
        expect(encrypted.type).toEqual(1);
        decrypted = aliceSession.decrypt(encrypted.type, encrypted.body);
        expect(decrypted).toEqual(TEST_TEXT);
        
        expect(bobSession.is_sender_chain_empty()).toEqual(false);
        expect(aliceSession.has_received_message()).toEqual(true);
    }); 
});
