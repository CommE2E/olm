"use strict";

var Olm = require('../olm');

describe("olm memory", function() {
    beforeEach(function(done) {
        Olm.init().then(function() {
            done();
        });
    });

    it('should return memory size', function() {
        // Total memory was set in Makefile
        const configured_total_memory = 262144; //256 * 1024
        expect(Olm.get_total_memory()).toEqual(configured_total_memory);

        const memory_before_creation = Olm.get_used_memory();
        var account = new Olm.Account();
        account.create();
        const memory_after_creation = Olm.get_used_memory();
        expect(memory_after_creation).toBeGreaterThan(memory_before_creation);

        account.free();
        const memory_after_freeing = Olm.get_used_memory();
        expect(memory_after_freeing).toEqual(memory_before_creation);
    });
});
