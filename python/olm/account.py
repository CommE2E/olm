import json
from os import urandom

from ._base import *

lib.olm_account_size.argtypes = []
lib.olm_account_size.restype = c_size_t

lib.olm_account.argtypes = [c_void_p]
lib.olm_account.restype = c_void_p

lib.olm_account_last_error.argtypes = [c_void_p]
lib.olm_account_last_error.restype = c_char_p

def account_errcheck(res, func, args):
    if res == ERR:
        raise OlmError("%s: %s" % (
            func.__name__, lib.olm_account_last_error(args[0])
        ))
    return res


def account_function(func, *types):
    func.argtypes = (c_void_p,) + types
    func.restypes = c_size_t
    func.errcheck = account_errcheck


account_function(
    lib.olm_pickle_account, c_void_p, c_size_t, c_void_p, c_size_t
)
account_function(
    lib.olm_unpickle_account, c_void_p, c_size_t, c_void_p, c_size_t
)
account_function(lib.olm_create_account_random_length)
account_function(lib.olm_create_account, c_void_p, c_size_t)
account_function(lib.olm_account_identity_keys_length)
account_function(lib.olm_account_identity_keys, c_void_p, c_size_t)
account_function(lib.olm_account_signature_length)
account_function(lib.olm_account_sign, c_void_p, c_size_t, c_void_p, c_size_t)
account_function(lib.olm_account_one_time_keys_length)
account_function(lib.olm_account_one_time_keys, c_void_p, c_size_t)
account_function(lib.olm_account_mark_keys_as_published)
account_function(lib.olm_account_max_number_of_one_time_keys)
account_function(lib.olm_pickle_account_length)
account_function(
    lib.olm_account_generate_one_time_keys_random_length,
    c_size_t
)
account_function(
    lib.olm_account_generate_one_time_keys,
    c_size_t,
    c_void_p, c_size_t
)
class Account(object):
    def __init__(self):
        self.buf = create_string_buffer(lib.olm_account_size())
        self.ptr = lib.olm_account(self.buf)

    def create(self):
        random_length = lib.olm_create_account_random_length(self.ptr)
        random = urandom(random_length)
        random_buffer = create_string_buffer(random)
        lib.olm_create_account(self.ptr, random_buffer, random_length)

    def pickle(self, key):
        key_buffer = create_string_buffer(key)
        pickle_length = lib.olm_pickle_account_length(self.ptr)
        pickle_buffer = create_string_buffer(pickle_length)
        lib.olm_pickle_account(
            self.ptr, key_buffer, len(key), pickle_buffer, pickle_length
        )
        return pickle_buffer.raw

    def unpickle(self, key, pickle):
        key_buffer = create_string_buffer(key)
        pickle_buffer = create_string_buffer(pickle)
        lib.olm_unpickle_account(
            self.ptr, key_buffer, len(key), pickle_buffer, len(pickle)
        )

    def identity_keys(self):
        out_length = lib.olm_account_identity_keys_length(self.ptr)
        out_buffer = create_string_buffer(out_length)
        lib.olm_account_identity_keys(
            self.ptr,
            out_buffer, out_length
        )
        return json.loads(out_buffer.raw)

    def sign(self, message):
        out_length = lib.olm_account_signature_length(self.ptr)
        message_buffer = create_string_buffer(message)
        out_buffer = create_string_buffer(out_length)
        lib.olm_account_sign(
            self.ptr, message_buffer, len(message), out_buffer, out_length
        )
        return out_buffer.raw

    def one_time_keys(self):
        out_length = lib.olm_account_one_time_keys_length(self.ptr)
        out_buffer = create_string_buffer(out_length)
        lib.olm_account_one_time_keys(self.ptr, out_buffer, out_length)
        return json.loads(out_buffer.raw)

    def mark_keys_as_published(self):
        lib.olm_account_mark_keys_as_published(self.ptr)

    def max_number_of_one_time_keys(self):
        return lib.olm_account_max_number_of_one_time_keys(self.ptr)

    def generate_one_time_keys(self, count):
        random_length = lib.olm_account_generate_one_time_keys_random_length(
            self.ptr, count
        )
        random = urandom(random_length)
        random_buffer = create_string_buffer(random)
        lib.olm_account_generate_one_time_keys(
            self.ptr, count, random_buffer, random_length
        )

    def clear(self):
        pass
