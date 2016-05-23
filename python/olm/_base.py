import os.path

from ctypes import *

def read_random(n):
    with open("/dev/urandom", "rb") as f:
        return f.read(n)

lib = cdll.LoadLibrary(os.path.join(
    os.path.dirname(__file__), "..", "..", "build", "libolm.so")
)

lib.olm_error.argtypes = []
lib.olm_error.restypes = c_size_t

ERR = lib.olm_error()

class OlmError(Exception):
    pass
