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

        const memory_before_creation = Olm.get_free_memory();
        var account = new Olm.Account();
        account.create();
        const memory_after_creation = Olm.get_free_memory();
        expect(memory_after_creation).toBeLessThan(memory_before_creation);

        account.free();
        const memory_after_freeing = Olm.get_free_memory();
        expect(memory_after_freeing).toEqual(memory_before_creation);
    });
});
