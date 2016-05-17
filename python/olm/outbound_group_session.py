import json

from ._base import *

lib.olm_outbound_group_session_size.argtypes = []
lib.olm_outbound_group_session_size.restype = c_size_t

lib.olm_outbound_group_session.argtypes = [c_void_p]
lib.olm_outbound_group_session.restype = c_void_p

lib.olm_outbound_group_session_last_error.argtypes = [c_void_p]
lib.olm_outbound_group_session_last_error.restype = c_char_p

def outbound_group_session_errcheck(res, func, args):
    if res == ERR:
        raise OlmError("%s: %s" % (
            func.__name__, lib.olm_outbound_group_session_last_error(args[0])
        ))
    return res


def outbound_group_session_function(func, *types):
    func.argtypes = (c_void_p,) + types
    func.restypes = c_size_t
    func.errcheck = outbound_group_session_errcheck


outbound_group_session_function(
    lib.olm_pickle_outbound_group_session, c_void_p, c_size_t, c_void_p, c_size_t
)
outbound_group_session_function(
    lib.olm_unpickle_outbound_group_session, c_void_p, c_size_t, c_void_p, c_size_t
)

outbound_group_session_function(lib.olm_init_outbound_group_session_random_length)
outbound_group_session_function(lib.olm_init_outbound_group_session, c_void_p, c_size_t)
outbound_group_session_function(lib.olm_group_encrypt_message_length, c_size_t)
outbound_group_session_function(lib.olm_group_encrypt,
    c_void_p, c_size_t,  # Plaintext
    c_void_p, c_size_t,  # Message
)


class OutboundGroupSession(object):
    def __init__(self):
        self.buf = create_string_buffer(lib.olm_outbound_group_session_size())
        self.ptr = lib.olm_outbound_group_session(self.buf)

        random_length = lib.olm_init_outbound_group_session_random_length(self.ptr)
        random = read_random(random_length)
        random_buffer = create_string_buffer(random)
        lib.olm_init_outbound_group_session(self.ptr, random_buffer, random_length)

    def pickle(self, key):
        key_buffer = create_string_buffer(key)
        pickle_length = lib.olm_pickle_outbound_group_session_length(self.ptr)
        pickle_buffer = create_string_buffer(pickle_length)
        lib.olm_pickle_outbound_group_session(
            self.ptr, key_buffer, len(key), pickle_buffer, pickle_length
        )
        return pickle_buffer.raw

    def unpickle(self, key, pickle):
        key_buffer = create_string_buffer(key)
        pickle_buffer = create_string_buffer(pickle)
        lib.olm_unpickle_outbound_group_session(
            self.ptr, key_buffer, len(key), pickle_buffer, len(pickle)
        )

    def encrypt(self, plaintext):
        message_length = lib.olm_group_encrypt_message_length(
            self.ptr, len(plaintext)
        )
        message_buffer = create_string_buffer(message_length)

        plaintext_buffer = create_string_buffer(plaintext)

        lib.olm_group_encrypt(
            self.ptr,
            plaintext_buffer, len(plaintext),
            message_buffer, message_length,
        )
        return message_buffer.raw
