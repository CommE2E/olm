"use strict";

var Olm = require('../olm');

describe("olm memory", function() {
    beforeEach(async function() {
        await Olm.init();
    });

    it('should return memory size and allocate more memory if needed', function() {
        // Total memory was set in Makefile
        const configured_total_memory = 262144; //256 * 1024
        expect(Olm.get_total_memory()).toEqual(configured_total_memory);

        // One `Account` object requires 7816 bytes of memory.
        // To test re-allocating memory, more than 262144 bytes are needed,
        // to achieve that create ~40 Accounts.
        const memory_before_creation = Olm.get_used_memory();

        let accounts = [];
        for (let it = 0; it < 40; it = it + 1) {
            const account = new Olm.Account();
            account.create();
            accounts.push(account);
        }

        const memory_after_creation = Olm.get_used_memory();
        expect(memory_after_creation).toBeGreaterThan(memory_before_creation);

        expect(Olm.get_total_memory()).toBeGreaterThan(configured_total_memory);

        for (let it = 0; it < 40; it = it + 1) {
            accounts[it].free();
        }

        const memory_after_freeing = Olm.get_used_memory();
        // The entire allocated memory was released.
        expect(memory_after_freeing).toEqual(memory_before_creation);

        // Not checking if total memory shrinks,
        // because this is not supported by Emscripten.
    });
});
