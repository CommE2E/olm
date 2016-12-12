var olm_exports = {};
var get_random_values;
var process; // Shadow the process object so that emscripten won't get
             // confused by browserify

if (typeof(window) !== 'undefined') {
    // We've in a browser (directly, via browserify, or via webpack).
    get_random_values = function(buf) {
        window.crypto.getRandomValues(buf);
    };
} else if (module["exports"]) {
    // We're running in node.
    var nodeCrypto = require("crypto");
    get_random_values = function(buf) {
        var bytes = nodeCrypto.randomBytes(buf.length);
        buf.set(bytes);
    };
    process = global["process"];
} else {
    throw new Error("Cannot find global to attach library to");
}

(function() {
    var module; // Shadow the Node 'module' object so that emscripten won't try
                // to fiddle with it.
