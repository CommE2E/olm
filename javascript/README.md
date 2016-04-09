Olm
===

Example:

    var alice = new Olm.Account();
    var bob = new Olm.Account();
    alice.create();
    bob.create();
    bob.generate_one_time_keys(1);

    var bobs_id_keys = JSON.parse(bob.identity_keys());
    var bobs_id_key = bobs_id_keys.curve25519;
    var bobs_ot_keys = JSON.parse(bob.one_time_keys());
    for (key in bobs_ot_keys.curve25519) {
        var bobs_ot_key = bobs_ot_keys.curve25519[key];
    }

    alice_session = new Olm.Session();
    alice_session.create_outbound(alice, bobs_id_key, bobs_ot_key);
    alice_message = a_session.encrypt("Hello");

    bob_session.create_inbound(bob, bob_message);
    var plaintext = bob_session.decrypt(message_1.type, bob_message);
    bob.remove_one_time_keys(bob_session);