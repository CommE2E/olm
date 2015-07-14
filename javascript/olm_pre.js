var olm_exports = {};
var get_random_values;
if (typeof(window) !== 'undefined') {
    // We've been imported directly into a browser.
    window["Olm"] = olm_exports;
    get_random_values = function(buf) {
        window.crypto.getRandomValues(buf);
    };
} else if (global.window) {
    // We're running with browserify
    global.window["Olm"] = olm_exports;
    get_random_values = function(buf) {
        window.crypto.getRandomValues(buf);
    };
} else if (module["exports"]) {
    // We're running in node.
    module["exports"] = olm_exports;
    var nodeCrypto = require("crypto");
    get_random_values = function(buf) {
        var bytes = nodeCrypto.randomBytes(buf.length);
        buf.set(bytes);
    }
} else {
    throw new Error("Cannot find global to attach library to");
}

var init = function() {
    var module; // Shadow the Node 'module' object so that emscripten won't try
                // to fiddle with it.
