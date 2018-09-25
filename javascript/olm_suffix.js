olm_exports['init'] = function() {
    return new Promise(function(resolve, reject) {
        onInitSuccess = function() {
            resolve();
        };
        onInitFail = function(err) {
            reject(err);
        };
        Module();
    });
};

if (typeof(window) !== 'undefined') {
    // We've been imported directly into a browser. Define the global 'Olm' object.
    // (we do this even if module.exports was defined, because it's useful to have
    // Olm in the global scope for browserified and webpacked apps.)
    window["Olm"] = olm_exports;
}

// Emscripten sets the module exports to be its module
// with wrapped c functions. Clobber it with our higher
// level wrapper class.
module.exports = olm_exports;
