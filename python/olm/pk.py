# -*- coding: utf-8 -*-
# libolm python bindings
# Copyright © 2018 Damir Jelić <poljar@termina.org.uk>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""libolm PK module.

This module contains bindings to the PK part of the Olm library.
It contains two classes PkDecryption and PkEncryption that are used to
establish an encrypted communication channel using public key encryption.

Examples:
    >>> decryption = PkDecryption()
    >>> encryption = PkEncryption(decryption.public_key)
    >>> plaintext = "It's a secret to everybody."
    >>> message = encryption.encrypt(plaintext)
    >>> decrypted_plaintext = decryption.decrypt(message)

"""

from builtins import super
from typing import AnyStr, Type
from future.utils import bytes_to_native_str

from _libolm import ffi, lib  # type: ignore
from ._finalize import track_for_finalization
from ._compat import URANDOM, to_bytearray


class PkEncryptionError(Exception):
    """libolm Pk encryption exception."""


class PkDecryptionError(Exception):
    """libolm Pk decryption exception."""


def _clear_pk_encryption(pk_struct):
    lib.olm_clear_pk_encryption(pk_struct)


class PkMessage(object):
    """A PK encrypted message."""

    def __init__(self, ephemeral_key, mac, ciphertext):
        # type: (str, str, str) -> None
        """Create a new PK encrypted message.

        Args:
            ephemeral_key(str): the public part of the ephemeral key
            used (together with the recipient's key) to generate a symmetric
            encryption key.
            mac(str): Message Authentication Code of the encrypted message
            ciphertext(str): The cipher text of the encrypted message
        """
        self.ephemeral_key = ephemeral_key
        self.mac = mac
        self.ciphertext = ciphertext


class PkEncryption(object):
    """PkEncryption class.

    Represents the decryption part of a PK encrypted channel.
    """

    def __init__(self, recipient_key):
        # type: (AnyStr) -> None
        """Create a new PK encryption object.

        Args:
            recipient_key(str): a public key that will be used for encryption
        """
        if not recipient_key:
            raise ValueError("Recipient key can't be empty")

        self._buf = ffi.new("char[]", lib.olm_pk_encryption_size())
        self._pk_encryption = lib.olm_pk_encryption(self._buf)
        track_for_finalization(self, self._pk_encryption, _clear_pk_encryption)

        byte_key = to_bytearray(recipient_key)
        lib.olm_pk_encryption_set_recipient_key(
            self._pk_encryption,
            ffi.from_buffer(byte_key),
            len(byte_key)
        )

        # clear out copies of the key
        if byte_key is not recipient_key:  # pragma: no cover
            for i in range(0, len(byte_key)):
                byte_key[i] = 0

    def _check_error(self, ret):  # pragma: no cover
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = bytes_to_native_str(
            ffi.string(lib.olm_pk_encryption_last_error(self._pk_encryption)))

        raise PkEncryptionError(last_error)

    def encrypt(self, plaintext):
        # type: (AnyStr) -> PkMessage
        """Encrypt a message.

        Returns the encrypted PkMessage.

        Args:
            plaintext(str): A string that will be encrypted using the
            PkEncryption object.
        """
        byte_plaintext = to_bytearray(plaintext)

        r_length = lib.olm_pk_encrypt_random_length(self._pk_encryption)
        random = URANDOM(r_length)
        random_buffer = ffi.new("char[]", random)

        ciphertext_length = lib.olm_pk_ciphertext_length(
            self._pk_encryption, len(byte_plaintext)
        )
        ciphertext = ffi.new("char[]", ciphertext_length)

        mac_length = lib.olm_pk_mac_length(self._pk_encryption)
        mac = ffi.new("char[]", mac_length)

        ephemeral_key_size = lib.olm_pk_key_length()
        ephemeral_key = ffi.new("char[]", ephemeral_key_size)

        ret = lib.olm_pk_encrypt(
            self._pk_encryption,
            ffi.from_buffer(byte_plaintext), len(byte_plaintext),
            ciphertext, ciphertext_length,
            mac, mac_length,
            ephemeral_key, ephemeral_key_size,
            random_buffer, r_length
        )

        try:
            self._check_error(ret)
        finally:  # pragma: no cover
            # clear out copies of plaintext
            if byte_plaintext is not plaintext:
                for i in range(0, len(byte_plaintext)):
                    byte_plaintext[i] = 0

        message = PkMessage(
            bytes_to_native_str(
                ffi.unpack(ephemeral_key, ephemeral_key_size)),
            bytes_to_native_str(
                ffi.unpack(mac, mac_length)),
            bytes_to_native_str(
                ffi.unpack(ciphertext, ciphertext_length))
        )
        return message


def _clear_pk_decryption(pk_struct):
    lib.olm_clear_pk_decryption(pk_struct)


class PkDecryption(object):
    """PkDecryption class.

    Represents the decryption part of a PK encrypted channel.

    Attributes:
        public_key (str): The public key of the PkDecryption object, can be
            shared and used to create a PkEncryption object.

    """

    def __new__(cls):
        # type: (Type[PkDecryption]) -> PkDecryption
        obj = super().__new__(cls)
        obj._buf = ffi.new("char[]", lib.olm_pk_decryption_size())
        obj._pk_decryption = lib.olm_pk_decryption(obj._buf)
        obj.public_key = None
        track_for_finalization(obj, obj._pk_decryption, _clear_pk_decryption)
        return obj

    def __init__(self):
        if False:  # pragma: no cover
            self._pk_decryption = self._pk_decryption  # type: ffi.cdata

        random_length = lib.olm_pk_private_key_length()
        random = URANDOM(random_length)
        random_buffer = ffi.new("char[]", random)

        key_length = lib.olm_pk_key_length()
        key_buffer = ffi.new("char[]", key_length)

        ret = lib.olm_pk_key_from_private(
            self._pk_decryption,
            key_buffer, key_length,
            random_buffer, random_length
        )
        self._check_error(ret)
        self.public_key = bytes_to_native_str(ffi.unpack(
            key_buffer,
            key_length
        ))

    def _check_error(self, ret):
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = bytes_to_native_str(
            ffi.string(lib.olm_pk_decryption_last_error(self._pk_decryption)))

        raise PkDecryptionError(last_error)

    def pickle(self, passphrase=""):
        # type: (str) -> bytes
        """Store a PkDecryption object.

        Stores a PkDecryption object as a base64 string. Encrypts the object
        using the supplied passphrase. Returns a byte object containing the
        base64 encoded string of the pickled session.

        Args:
            passphrase(str, optional): The passphrase to be used to encrypt
                the object.
        """
        byte_key = to_bytearray(passphrase)

        pickle_length = lib.olm_pickle_pk_decryption_length(
            self._pk_decryption
        )
        pickle_buffer = ffi.new("char[]", pickle_length)

        ret = lib.olm_pickle_pk_decryption(
            self._pk_decryption,
            ffi.from_buffer(byte_key), len(byte_key),
            pickle_buffer, pickle_length
        )
        try:
            self._check_error(ret)
        finally:
            # zero out copies of the passphrase
            for i in range(0, len(byte_key)):
                byte_key[i] = 0

        return ffi.unpack(pickle_buffer, pickle_length)

    @classmethod
    def from_pickle(cls, pickle, passphrase=""):
        # types: (bytes, str) -> PkDecryption
        """Restore a previously stored PkDecryption object.

        Creates a PkDecryption object from a pickled base64 string. Decrypts
        the pickled object using the supplied passphrase.
        Raises PkDecryptionError on failure. If the passphrase
        doesn't match the one used to encrypt the session then the error
        message for the exception will be "BAD_ACCOUNT_KEY". If the base64
        couldn't be decoded then the error message will be "INVALID_BASE64".

        Args:
            pickle(bytes): Base64 encoded byte string containing the pickled
                PkDecryption object
            passphrase(str, optional): The passphrase used to encrypt the
                object
        """
        if not pickle:
            raise ValueError("Pickle can't be empty")

        byte_key = to_bytearray(passphrase)
        pickle_buffer = ffi.new("char[]", pickle)

        pubkey_length = lib.olm_pk_key_length()
        pubkey_buffer = ffi.new("char[]", pubkey_length)

        obj = cls.__new__(cls)

        ret = lib.olm_unpickle_pk_decryption(
            obj._pk_decryption,
            ffi.from_buffer(byte_key), len(byte_key),
            pickle_buffer, len(pickle),
            pubkey_buffer, pubkey_length)

        try:
            obj._check_error(ret)
        finally:
            for i in range(0, len(byte_key)):
                byte_key[i] = 0

        obj.public_key = bytes_to_native_str(ffi.unpack(
            pubkey_buffer,
            pubkey_length
        ))

        return obj

    def decrypt(self, message):
        # type (PkMessage) -> str
        """Decrypt a previously encrypted Pk message.

        Returns the decrypted plaintext.
        Raises PkDecryptionError on failure.

        Args:
            message(PkMessage): the pk message to decrypt.
        """
        ephemeral_key = to_bytearray(message.ephemeral_key)
        ephemeral_key_size = len(ephemeral_key)

        mac = to_bytearray(message.mac)
        mac_length = len(mac)

        ciphertext = to_bytearray(message.ciphertext)
        ciphertext_length = len(ciphertext)

        max_plaintext_length = lib.olm_pk_max_plaintext_length(
            self._pk_decryption,
            ciphertext_length
        )
        plaintext_buffer = ffi.new("char[]", max_plaintext_length)

        ret = lib.olm_pk_decrypt(
            self._pk_decryption,
            ffi.from_buffer(ephemeral_key), ephemeral_key_size,
            ffi.from_buffer(mac), mac_length,
            ffi.from_buffer(ciphertext), ciphertext_length,
            plaintext_buffer, max_plaintext_length)
        self._check_error(ret)

        plaintext = (ffi.unpack(
            plaintext_buffer,
            ret
        ))

        # clear out copies of the plaintext
        lib.memset(plaintext_buffer, 0, max_plaintext_length)

        return bytes_to_native_str(plaintext)
