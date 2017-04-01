from os import urandom

from ._base import *


lib.olm_session_size.argtypes = []
lib.olm_session_size.restype = c_size_t

lib.olm_session.argtypes = [c_void_p]
lib.olm_session.restype = c_void_p

lib.olm_session_last_error.argtypes = [c_void_p]
lib.olm_session_last_error.restype = c_char_p


def session_errcheck(res, func, args):
    if res == ERR:
        raise OlmError("%s: %s" % (
            func.__name__, lib.olm_session_last_error(args[0])
        ))
    return res


def session_function(func, *types):
    func.argtypes = (c_void_p,) + types
    func.restypes = c_size_t
    func.errcheck = session_errcheck

session_function(lib.olm_session_last_error)
session_function(
    lib.olm_pickle_session, c_void_p, c_size_t, c_void_p, c_size_t
)
session_function(
    lib.olm_unpickle_session, c_void_p, c_size_t, c_void_p, c_size_t
)
session_function(lib.olm_create_outbound_session_random_length)
session_function(
    lib.olm_create_outbound_session,
    c_void_p,  # Account
    c_void_p, c_size_t,  # Identity Key
    c_void_p, c_size_t,  # One Time Key
    c_void_p, c_size_t,  # Random
)
session_function(
    lib.olm_create_inbound_session,
    c_void_p,  # Account
    c_void_p, c_size_t,  # Pre Key Message
)
session_function(
    lib.olm_create_inbound_session_from,
    c_void_p,  # Account
    c_void_p, c_size_t,  # Identity Key
    c_void_p, c_size_t,  # Pre Key Message
)
session_function(lib.olm_session_id_length)
session_function(lib.olm_session_id, c_void_p, c_size_t)
session_function(lib.olm_matches_inbound_session, c_void_p, c_size_t)
session_function(
    lib.olm_matches_inbound_session_from,
    c_void_p, c_size_t,  # Identity Key
    c_void_p, c_size_t,  # Pre Key Message
)
session_function(lib.olm_pickle_session_length)
session_function(lib.olm_encrypt_message_type)
session_function(lib.olm_encrypt_random_length)
session_function(lib.olm_encrypt_message_length, c_size_t)
session_function(
    lib.olm_encrypt,
    c_void_p, c_size_t,  # Plaintext
    c_void_p, c_size_t,  # Random
    c_void_p, c_size_t,  # Message
);
session_function(
    lib.olm_decrypt_max_plaintext_length,
    c_size_t,  # Message Type
    c_void_p, c_size_t,  # Message
)
session_function(
    lib.olm_decrypt,
    c_size_t,  # Message Type
    c_void_p, c_size_t,  # Message
    c_void_p, c_size_t,  # Plaintext
)

class Session(object):
    def __init__(self):
        self.buf = create_string_buffer(lib.olm_session_size())
        self.ptr = lib.olm_session(self.buf)

    def pickle(self, key):
        key_buffer = create_string_buffer(key)
        pickle_length = lib.olm_pickle_session_length(self.ptr)
        pickle_buffer = create_string_buffer(pickle_length)
        lib.olm_pickle_session(
            self.ptr, key_buffer, len(key), pickle_buffer, pickle_length
        )
        return pickle_buffer.raw

    def unpickle(self, key, pickle):
        key_buffer = create_string_buffer(key)
        pickle_buffer = create_string_buffer(pickle)
        lib.olm_unpickle_session(
            self.ptr, key_buffer, len(key), pickle_buffer, len(pickle)
        )

    def create_outbound(self, account, identity_key, one_time_key):
        r_length = lib.olm_create_outbound_session_random_length(self.ptr)
        random = urandom(r_length)
        random_buffer = create_string_buffer(random)
        identity_key_buffer = create_string_buffer(identity_key)
        one_time_key_buffer = create_string_buffer(one_time_key)
        lib.olm_create_outbound_session(
            self.ptr,
            account.ptr,
            identity_key_buffer, len(identity_key),
            one_time_key_buffer, len(one_time_key),
            random_buffer, r_length
        )

    def create_inbound(self, account, one_time_key_message):
        one_time_key_message_buffer = create_string_buffer(one_time_key_message)
        lib.olm_create_inbound_session(
            self.ptr,
            account.ptr,
            one_time_key_message_buffer, len(one_time_key_message)
        )

    def create_inbound_from(self, account, identity_key, one_time_key_message):
        identity_key_buffer = create_string_buffer(identity_key)
        one_time_key_message_buffer = create_string_buffer(one_time_key_message)
        lib.olm_create_inbound_session_from(
            self.ptr,
            account.ptr,
            identity_key_buffer, len(identity_key),
            one_time_key_message_buffer, len(one_time_key_message)
        )

    def session_id(self):
        id_length = lib.olm_session_id_length(self.ptr)
        id_buffer = create_string_buffer(id_length)
        lib.olm_session_id(self.ptr, id_buffer, id_length);
        return id_buffer.raw

    def matches_inbound(self, one_time_key_message):
        one_time_key_message_buffer = create_string_buffer(one_time_key_message)
        return bool(lib.olm_matches_inbound_session(
            self.ptr,
            one_time_key_message_buffer, len(one_time_key_message)
        ))

    def matches_inbound_from(self, identity_key, one_time_key_message):
        identity_key_buffer = create_string_buffer(identity_key)
        one_time_key_message_buffer = create_string_buffer(one_time_key_message)
        return bool(lib.olm_matches_inbound_session(
            self.ptr,
            identity_key_buffer, len(identity_key),
            one_time_key_message_buffer, len(one_time_key_message)
        ))

    def encrypt(self, plaintext):
        r_length = lib.olm_encrypt_random_length(self.ptr)
        random = urandom(r_length)
        random_buffer = create_string_buffer(random)

        message_type = lib.olm_encrypt_message_type(self.ptr)
        message_length = lib.olm_encrypt_message_length(
            self.ptr, len(plaintext)
        )
        message_buffer = create_string_buffer(message_length)

        plaintext_buffer = create_string_buffer(plaintext)

        lib.olm_encrypt(
            self.ptr,
            plaintext_buffer, len(plaintext),
            random_buffer, r_length,
            message_buffer, message_length,
        )
        return message_type, message_buffer.raw

    def decrypt(self, message_type, message):
        message_buffer = create_string_buffer(message)
        max_plaintext_length = lib.olm_decrypt_max_plaintext_length(
            self.ptr, message_type, message_buffer, len(message)
        )
        plaintext_buffer = create_string_buffer(max_plaintext_length)
        message_buffer = create_string_buffer(message)
        plaintext_length = lib.olm_decrypt(
            self.ptr, message_type, message_buffer, len(message),
            plaintext_buffer, max_plaintext_length
        )
        return plaintext_buffer.raw[:plaintext_length]

    def clear(self):
        pass
