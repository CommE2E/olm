
function OutboundGroupSession() {
    var size = Module['_olm_outbound_group_session_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_outbound_group_session'](this.buf);
}

function outbound_group_session_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = Pointer_stringify(
                Module['_olm_outbound_group_session_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

OutboundGroupSession.prototype['free'] = function() {
    Module['_olm_clear_outbound_group_session'](this.ptr);
    free(this.ptr);
}

OutboundGroupSession.prototype['pickle'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var pickle_length = outbound_group_session_method(
        Module['_olm_pickle_outbound_group_session_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length);
    outbound_group_session_method(Module['_olm_pickle_outbound_group_session'])(
        this.ptr, key_buffer, key_array.length, pickle_buffer, pickle_length
    );
    return Pointer_stringify(pickle_buffer, pickle_length);
});

OutboundGroupSession.prototype['unpickle'] = restore_stack(function(key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_array);
    outbound_group_session_method(Module['_olm_unpickle_outbound_group_session'])(
        this.ptr, key_buffer, key_array.length, pickle_buffer,
        pickle_array.length
    );
});

OutboundGroupSession.prototype['create'] = restore_stack(function(key) {
    var random_length = session_method(
        Module['_olm_init_outbound_group_session_random_length']
    )(this.ptr);
    var random = random_stack(random_length);
    outbound_group_session_method(Module['_olm_init_outbound_group_session'])(
        this.ptr, random, random_length
    );
});

OutboundGroupSession.prototype['encrypt'] = restore_stack(function(plaintext) {
    var plaintext_array = array_from_string(plaintext);
    var message_length = outbound_group_session_method(
        Module['_olm_group_encrypt_message_length']
    )(this.ptr, plaintext_array.length);
    var plaintext_buffer = stack(plaintext_array);
    var message_buffer = stack(message_length);
    outbound_group_session_method(Module['_olm_group_encrypt'])(
        this.ptr,
        plaintext_buffer, plaintext_array.length,
        message_buffer, message_length
    );
    return Pointer_stringify(message_buffer, message_length);
});

OutboundGroupSession.prototype['session_id'] = restore_stack(function(key) {
    var length = outbound_group_session_method(
        Module['_olm_outbound_group_session_id_length']
    )(this.ptr);
    var session_id = stack(length);
    outbound_group_session_method(Module['_olm_outbound_group_session_id'])(
        this.ptr, session_id, length
    );
    return Pointer_stringify(session_id, length);
});

OutboundGroupSession.prototype['session_key'] = restore_stack(function(key) {
    var key_length = outbound_group_session_method(
        Module['_olm_outbound_group_session_key_length']
    )(this.ptr);
    var key = stack(key_length);
    outbound_group_session_method(Module['_olm_outbound_group_session_key'])(
        this.ptr, key, key_length
    );
    return Pointer_stringify(key, key_length);
});

OutboundGroupSession.prototype['message_index'] = function() {
    var idx = outbound_group_session_method(
        Module['_olm_outbound_group_session_message_index']
    )(this.ptr);
    return idx;
};

olm_exports['OutboundGroupSession'] = OutboundGroupSession;
