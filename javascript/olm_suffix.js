var olmInitPromise;

olm_exports['init'] = function(opts) {
    if (olmInitPromise) return olmInitPromise;

    if (opts) OLM_OPTIONS = opts;

    olmInitPromise = new Promise(function(resolve, reject) {
        onInitSuccess = function() {
            resolve();
        };
        onInitFail = function(err) {
            reject(err);
        };
        Module();
    });
    return olmInitPromise;
};

if (typeof(window) !== 'undefined') {
    // We've been imported directly into a browser. Define the global 'Olm' object.
    // (we do this even if module.exports was defined, because it's useful to have
    // Olm in the global scope for browserified and webpacked apps.)
    window["Olm"] = olm_exports;
}

if (typeof module === 'object') {
    // Emscripten sets the module exports to be its module
    // with wrapped c functions. Clobber it with our higher
    // level wrapper class.
    module.exports = olm_exports;
}
