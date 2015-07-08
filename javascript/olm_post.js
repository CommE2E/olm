var runtime = Module['Runtime'];
var malloc = Module['_malloc'];
var free = Module['_free'];
var Pointer_stringify = Module['Pointer_stringify'];
var OLM_ERROR = Module['_olm_error']();

function stack(size_or_array) {
    return Module['allocate'](size_or_array, 'i8', Module['ALLOC_STACK']);
}

function array_from_string(string) {
    return Module['intArrayFromString'](string, true);
}

function random_stack(size) {
    var ptr = stack(size);
    var array = new Uint8Array(Module['HEAPU8'].buffer, ptr, size);
    window.crypto.getRandomValues(array);
    return ptr;
}

function restore_stack(wrapped) {
    return function() {
        var sp = runtime.stackSave();
        try {
            return wrapped.apply(this, arguments);
        } finally {
            runtime.stackRestore(sp);
        }
    }
}

function Account() {
    var size = Module['_olm_account_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_account'](this.buf);
}

function account_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = Pointer_stringify(
                Module['_olm_account_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

Account.prototype['free'] = function() {
    free(this.ptr);
}

Account.prototype['create'] = restore_stack(function() {
    var random_length = account_method(
        Module['_olm_create_account_random_length']
    )(this.ptr);
    var random = random_stack(random_length);
    account_method(Module['_olm_create_account'])(
        this.ptr, random, random_length
    );
});

Account.prototype['identity_keys'] = restore_stack(function(
    user_id, device_id, valid_after, valid_until
) {
    var user_id_array = array_from_string(user_id);
    var device_id_array = array_from_string(device_id);
    var keys_length = account_method(
        Module['_olm_account_identity_keys_length']
    )(
        this.ptr, user_id_array.length, device_id_array.length,
        valid_after, valid_after / Math.pow(2, 32),
        valid_until, valid_until / Math.pow(2, 32)
    );
    var user_id_buffer = stack(user_id_array);
    var device_id_buffer = stack(device_id_array);
    var keys = stack(keys_length);
    account_method(Module['_olm_account_identity_keys'])(
        this.ptr,
        user_id_buffer, user_id_array.length,
        device_id_buffer, device_id_array.length,
        valid_after, valid_after / Math.pow(2, 32),
        valid_until, valid_until / Math.pow(2, 32),
        keys, keys_length
    );
    return Pointer_stringify(keys, keys_length);
});

Account.prototype['one_time_keys'] = restore_stack(function() {
    var keys_length = account_method(
        Module['_olm_account_one_time_keys_length']
    )(this.ptr);
    var keys = stack(keys_length);
    account_method(Module['_olm_account_one_time_keys'])(
        this.ptr, keys, keys_length
    );
    return Pointer_stringify(keys, keys_length);
});

Account.prototype['pickle'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var pickle_length = account_method(
        Module['_olm_pickle_account_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length);
    account_method(Module['_olm_pickle_account'])(
        this.ptr, key_buffer, key_array.length, pickle_buffer, pickle_length
    );
    return Pointer_stringify(pickle_buffer, pickle_length);
});

Account.prototype['unpickle'] = restore_stack(function(key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_length);
    account_method(Module['_olm_unpickle_account'])(
        this.ptr, key_buffer, key_array.length, pickle_buffer,
        pickle_array.length
    );
});

function Session() {
    var size = Module['_olm_session_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_session'](this.buf);
}

function session_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = Pointer_stringify(
                Module['_olm_session_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

Session.prototype['free'] = function() {
    free(this.ptr);
}

Session.prototype['pickle'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var pickle_length = session_method(
        Module['_olm_pickle_session_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length);
    session_method(Module['_olm_pickle_session'])(
        this.ptr, key_buffer, key_array.length, pickle_buffer, pickle_length
    );
    return Pointer_stringify(pickle_buffer, pickle_length);
});

Session.prototype['unpickle'] = restore_stack(function(key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_array);
    session_method(Module['_olm_unpickle_session'])(
        this.ptr, key_buffer, key_array.length, pickle_buffer,
        pickle_array.length
    );
});

Session.prototype['create_outbound'] = restore_stack(function(
    account, their_identity_key, their_one_time_key
) {
    var random_length = session_method(
        Module['_olm_create_outbound_session_random_length']
    )(this.ptr);
    var random = random_stack(random_length);
    var identity_key_array = array_from_string(their_identity_key);
    var one_time_key_array = array_from_string(their_one_time_key);
    var identity_key_buffer = stack(identity_key_array);
    var one_time_key_buffer = stack(one_time_key_array);
    session_method(Module['_olm_create_outbound_session'])(
        this.ptr, account.ptr,
        identity_key_buffer, identity_key_array.length,
        one_time_key_buffer, one_time_key_array.length,
        random, random_length
    );
});

Session.prototype['create_inbound'] = restore_stack(function(
    account, one_time_key_message
) {
    var message_array = array_from_string(one_time_key_message);
    var message_buffer = stack(message_array);
    session_method(Module['_olm_create_inbound_session'])(
        this.ptr, account.ptr, message_buffer, message_array.length
    );
});

Session.prototype['matches_inbound'] = restore_stack(function(
    account, one_time_key_message
) {
    var message_array = array_from_string(one_time_key_message);
    var message_buffer = stack(message_array);
    return session_method(Module['_olm_matches_inbound_session'])(
        this.ptr, account.ptr, message_buffer, message_array.length
    ) ? true : false;
});

Session.prototype['encrypt'] = restore_stack(function(
    plaintext
) {
    var random_length = session_method(
        Module['_olm_encrypt_random_length']
    )(this.ptr);
    var message_type = session_method(
        Module['_olm_encrypt_message_type']
    )(this.ptr);
    var plaintext_array = array_from_string(plaintext);
    var message_length = session_method(
        Module['_olm_encrypt_message_length']
    )(this.ptr, plaintext_array.length);
    var random = random_stack(random_length);
    var plaintext_buffer = stack(plaintext_array);
    var message_buffer = stack(message_length);
    session_method(Module['_olm_encrypt'])(
        this.ptr,
        plaintext_buffer, plaintext_array.length,
        random, random_length,
        message_buffer, message_length
    );
    return {
        "type": message_type,
        "body": Pointer_stringify(message_buffer, message_length)
    };
});

Session.prototype['decrypt'] = restore_stack(function(
    message_type, message
) {
    var message_array = array_from_string(message);
    var message_buffer = stack(message_array);
    var max_plaintext_length = session_method(
        Module['_olm_decrypt_max_plaintext_length']
    )(this.ptr, message_type, message_buffer, message_array.length);
    // caculating the length destroys the input buffer.
    // So we copy the array to a new buffer
    var message_buffer = stack(message_array);
    var plaintext_buffer = stack(max_plaintext_length);
    var plaintext_length = session_method(Module["_olm_decrypt"])(
        this.ptr, message_type,
        message_buffer, message.length,
        plaintext_buffer, max_plaintext_length
    );
    return Pointer_stringify(plaintext_buffer, plaintext_length);
});

return {"Account": Account, "Session": Session};
}();
