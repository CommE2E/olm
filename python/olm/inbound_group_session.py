import json

from ._base import *

lib.olm_inbound_group_session_size.argtypes = []
lib.olm_inbound_group_session_size.restype = c_size_t

lib.olm_inbound_group_session.argtypes = [c_void_p]
lib.olm_inbound_group_session.restype = c_void_p

lib.olm_inbound_group_session_last_error.argtypes = [c_void_p]
lib.olm_inbound_group_session_last_error.restype = c_char_p

def inbound_group_session_errcheck(res, func, args):
    if res == ERR:
        raise OlmError("%s: %s" % (
            func.__name__, lib.olm_inbound_group_session_last_error(args[0])
        ))
    return res


def inbound_group_session_function(func, *types):
    func.argtypes = (c_void_p,) + types
    func.restypes = c_size_t
    func.errcheck = inbound_group_session_errcheck


inbound_group_session_function(
    lib.olm_pickle_inbound_group_session, c_void_p, c_size_t, c_void_p, c_size_t
)
inbound_group_session_function(
    lib.olm_unpickle_inbound_group_session, c_void_p, c_size_t, c_void_p, c_size_t
)

inbound_group_session_function(
    lib.olm_init_inbound_group_session, c_uint32, c_void_p, c_size_t
)

inbound_group_session_function(
    lib.olm_group_decrypt_max_plaintext_length, c_void_p, c_size_t
)
inbound_group_session_function(
    lib.olm_group_decrypt,
    c_void_p, c_size_t, # message
    c_void_p, c_size_t, # plaintext
)

inbound_group_session_function(lib.olm_inbound_group_session_id_length)
inbound_group_session_function(lib.olm_inbound_group_session_id, c_void_p, c_size_t)

class InboundGroupSession(object):
    def __init__(self):
        self.buf = create_string_buffer(lib.olm_inbound_group_session_size())
        self.ptr = lib.olm_inbound_group_session(self.buf)

    def pickle(self, key):
        key_buffer = create_string_buffer(key)
        pickle_length = lib.olm_pickle_inbound_group_session_length(self.ptr)
        pickle_buffer = create_string_buffer(pickle_length)
        lib.olm_pickle_inbound_group_session(
            self.ptr, key_buffer, len(key), pickle_buffer, pickle_length
        )
        return pickle_buffer.raw

    def unpickle(self, key, pickle):
        key_buffer = create_string_buffer(key)
        pickle_buffer = create_string_buffer(pickle)
        lib.olm_unpickle_inbound_group_session(
            self.ptr, key_buffer, len(key), pickle_buffer, len(pickle)
        )

    def init(self, message_index, session_key):
        key_buffer = create_string_buffer(session_key)
        lib.olm_init_inbound_group_session(
            self.ptr, message_index, key_buffer, len(session_key)
        )

    def decrypt(self, message):
        message_buffer = create_string_buffer(message)
        max_plaintext_length = lib.olm_group_decrypt_max_plaintext_length(
            self.ptr, message_buffer, len(message)
        )
        plaintext_buffer = create_string_buffer(max_plaintext_length)
        message_buffer = create_string_buffer(message)
        plaintext_length = lib.olm_group_decrypt(
            self.ptr, message_buffer, len(message),
            plaintext_buffer, max_plaintext_length
        )
        return plaintext_buffer.raw[:plaintext_length]

    def session_id(self):
        id_length = lib.olm_inbound_group_session_id_length(self.ptr)
        id_buffer = create_string_buffer(id_length)
        lib.olm_inbound_group_session_id(self.ptr, id_buffer, id_length);
        return id_buffer.raw
