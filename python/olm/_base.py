import os.path

from ctypes import *


lib = cdll.LoadLibrary(os.path.join(
    os.path.dirname(__file__), "..", "..", "build", "libolm.so.2")
)

lib.olm_error.argtypes = []
lib.olm_error.restypes = c_size_t

ERR = lib.olm_error()

class OlmError(Exception):
    pass
