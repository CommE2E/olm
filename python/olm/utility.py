from ._base import lib, c_void_p, c_size_t, c_char_p, \
    create_string_buffer, ERR, OlmError

lib.olm_utility_size.argtypes = []
lib.olm_utility_size.restype = c_size_t

lib.olm_utility.argtypes = [c_void_p]
lib.olm_utility.restype = c_void_p

lib.olm_utility_last_error.argtypes = [c_void_p]
lib.olm_utility_last_error.restype = c_char_p


def utility_errcheck(res, func, args):
    if res == ERR:
        raise OlmError("%s: %s" % (
            func.__name__, lib.olm_utility_last_error(args[0])
        ))
    return res


def utility_function(func, *types):
    func.argtypes = (c_void_p,) + types
    func.restypes = c_size_t
    func.errcheck = utility_errcheck

utility_function(
    lib.olm_ed25519_verify,
    c_void_p, c_size_t,  # key, key_length
    c_void_p, c_size_t,  # message, message_length
    c_void_p, c_size_t,  # signature, signature_length
)


class Utility(object):
    def __init__(self):
        self.buf = create_string_buffer(lib.olm_utility_size())
        self.ptr = lib.olm_utility(self.buf)

_utility = None


def ed25519_verify(key, message, signature):
    """ Verify an ed25519 signature. Raises an OlmError if verification fails.
    Args:
        key(bytes): The ed25519 public key used for signing.
        message(bytes): The signed message.
        signature(bytes): The message signature.
    """
    global _utility
    if not _utility:
        _utility = Utility()
    lib.olm_ed25519_verify(_utility.ptr,
                           key, len(key),
                           message, len(message),
                           signature, len(signature))
