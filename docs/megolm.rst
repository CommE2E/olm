Megolm group ratchet
====================

An AES-based cryptographic ratchet intended for group communications.

Background
----------

The Megolm ratchet is intended for encrypted messaging applications where there
may be a large number of recipients of each message, thus precluding the use of
peer-to-peer encryption systems such as `Olm`_.

It also allows a receipient to decrypt received messages multiple times. For
instance, in client/server applications, a copy of the ciphertext can be stored
on the (untrusted) server, while the client need only store the session keys.

Overview
--------

Each participant in a conversation uses their own session, which consists of a
ratchet, and an `Ed25519`_ keypair.

Secrecy is provided by the ratchet, which can be wound forwards, via hash
functions, but not backwards, and is used to derive a distinct message key
for each message.

Authenticity is provided via the Ed25519 key.

The value of the ratchet, and the public part of the Ed25519 key, are shared
with other participants in the conversation via secure peer-to-peer
channels. Provided that peer-to-peer channel provides authenticity of the
messages to the participants and deniability of the messages to third parties,
the Megolm session will inherit those properties.

The Megolm algorithm
--------------------

Initial setup
~~~~~~~~~~~~~

Each participant in a conversation generates their own Megolm session. A
session consists of three parts: a 32 bit counter, :math:`i`; an `Ed25519`_
keypair, :math:`K`; and a ratchet, :math:`R_i`. The ratchet consists of four
256-bit values, :math:`R_{i,j}` for :math:`j \in {0,1,2,3}`.

The counter :math:`i` is initialised to :math:`0`. A new Ed25519 keypair is
generated for :math:`K`. The ratchet is simply initialised with 1024 bits of
cryptographically-secure random data.

A single participant may use multiple sessions over the lifetime of a
conversation. The public part of :math:`K` is used as an identifier to
discriminate between sessions.

Sharing session data
~~~~~~~~~~~~~~~~~~~~

To allow other participants in the conversation to decrypt messages, the
session data is formatted as described in `Session-sharing format`_. It is then
shared with other participants in the conversation via a secure peer-to-peer
channel (such as that provided by `Olm`_).

When the session data is received from other participants, the recipient first
checks that the signature matches the public key. They then store their own
copy of the counter, ratchet, and public key.

Message encryption
~~~~~~~~~~~~~~~~~~

Megolm uses AES-256_ in CBC_ mode with `PCKS#7`_ padding for and HMAC-SHA-256_
(truncated to 64 bits).  The 256 bit AES key, 256 bit HMAC key, and 128 bit AES
IV are derived from the megolm ratchet :math:`R_i`:

.. math::

    \begin{align}
    AES\_KEY_{i}\;\parallel\;HMAC\_KEY_{i}\;\parallel\;AES\_IV_{i}
        &= HKDF\left(0,\,R_{i},\text{"MEGOLM\_KEYS"},\,80\right) \\
    \end{align}

where :math:`\parallel` represents string splitting, and
:math:`HKDF\left(salt,\,IKM,\,info,\,L\right)` refers to the `HMAC-based key
derivation function`_ using using `SHA-256`_ as the hash function
(`HKDF-SHA-256`_) with a salt value of :math:`salt`, input key material of
:math:`IKM`, context string :math:`info`, and output keying material length of
:math:`L` bytes.

The plain-text is encrypted with AES-256, using the key :math:`AES\_KEY_{i}`
and the IV :math:`AES\_IV_{i}` to give the cipher-text, :math:`X_{i}`.

The ratchet index :math:`i`, and the cipher-text :math:`X_{i}`, are then packed
into a message as described in `Message format`_. Then the entire message
(including the version bytes and all payload bytes) are passed through
HMAC-SHA-256. The first 8 bytes of the MAC are appended to the message.

Finally, the authenticated message is signed using the Ed25519 keypair; the 64
byte signature is appended to the message.

The complete signed message, together with the public part of :math:`K` (acting
as a session identifier), can then be sent over an insecure channel. The
message can then be authenticated and decrypted only by recipients who have
received the session data.

Advancing the ratchet
~~~~~~~~~~~~~~~~~~~~~

After each message is encrypted, the ratchet is advanced. This is done as
follows:

.. math::
    \begin{align}
    R_{i,0} &=
      \begin{cases}
        HMAC\left(R_{2^24(n-1),0}, \text{"\textbackslash x00"}\right)
          &\text{if }\exists n | i = 2^24n\\
        R_{i-1,0} &\text{otherwise}
      \end{cases}\\
    R_{i,1} &=
      \begin{cases}
        HMAC\left(R_{2^24(n-1),0}, \text{"\textbackslash x01"}\right)
          &\text{if }\exists n | i = 2^24n\\
        HMAC\left(R_{2^16(m-1),1}, \text{"\textbackslash x01"}\right)
          &\text{if }\exists m | i = 2^16m\\
        R_{i-1,1} &\text{otherwise}
      \end{cases}\\
    R_{i,2} &=
      \begin{cases}
        HMAC\left(R_{2^24(n-1),0}, \text{"\textbackslash x02"}\right)
          &\text{if }\exists n | i = 2^24n\\
        HMAC\left(R_{2^16(m-1),1}, \text{"\textbackslash x02"}\right)
          &\text{if }\exists m | i = 2^16m\\
        HMAC\left(R_{2^8(p-1),2}, \text{"\textbackslash x02"}\right)
          &\text{if }\exists p | i = 2^8p\\
        R_{i-1,2} &\text{otherwise}
      \end{cases}\\
    R_{i,3} &=
      \begin{cases}
        HMAC\left(R_{2^24(n-1),0}, \text{"\textbackslash x03"}\right)
          &\text{if }\exists n | i = 2^24n\\
        HMAC\left(R_{2^16(m-1),1}, \text{"\textbackslash x03"}\right)
          &\text{if }\exists m | i = 2^16m\\
        HMAC\left(R_{2^8(p-1),2}, \text{"\textbackslash x03"}\right)
          &\text{if }\exists p | i = 2^8p\\
        HMAC\left(R_{i-1,3}, \text{"\textbackslash x03"}\right)
          &\text{otherwise}
      \end{cases}
    \end{align}

where :math:`HMAC(K, T)` is the HMAC-SHA-256_ of ``T``, using ``K`` as the
key. In summary: every :math:`2^8` iterations, :math:`R_{i,3}` is reseeded from
:math:`R_{i,2}`. Every :math:`2^16` iterations, :math:`R_{i,2}` and
:math:`R_{i,3}` are reseeded from :math:`R_{i,1}`. Every :math:`2^24`
iterations, :math:`R_{i,1}`, :math:`R_{i,2}` and :math:`R_{i,3}` are reseeded
from :math:`R_{i,0}`.

This scheme allows the ratchet to be advanced an arbitrary amount forwards
while needing at most 1023 hash computations. A recipient can decrypt
conversation history onwards from the earliest value of the ratchet it is aware
of, but cannot decrypt history from before that point without reversing the
hash function.

For outbound sessions, the updated ratchet and counter are stored in the
session.

In order to maintain the ability to decrypt conversation history, inbound
sessions should store a copy of their earliet known ratchet value (unless they
explicitly want to drop the ability to decrypt that history). They may also
choose to cache calculated ratchet values, but the decision of which ratchet
states to cache is left to the application.

Data exchange formats
---------------------

Session-sharing format
~~~~~~~~~~~~~~~~~~~~~~

The Megolm key-sharing format is as follows:

.. code::

    +---+----+--------+--------+--------+--------+------+-----------+
    | V | i  | R(i,0) | R(i,1) | R(i,2) | R(i,3) | Kpub | Signature |
    +---+----+--------+--------+--------+--------+------+-----------+
    0   1    5        37       69      101      133    165         229

The version byte, ``V``, is ``"\x02"``.

This is followed by the ratchet index, :math:`i`, which is encoded as a
big-endian 32-bit integer; the ratchet values :math:`R_{i,j}`; and the public
part of the Ed25519 keypair :math:`K`.

The data is then signed using the Ed25519 keypair, and the 64-byte signature is
appended.

Message format
~~~~~~~~~~~~~~

Megolm messages consist of a one byte version, followed by a variable length
payload, a fixed length message authentication code, and a fixed length
signature.

.. code::

   +---+------------------------------------+-----------+------------------+
   | V | Payload Bytes                      | MAC Bytes | Signature Bytes  |
   +---+------------------------------------+-----------+------------------+
   0   1                                    N          N+8                N+72

The version byte, ``V``, is ``"\x03"``.

The payload consists of key-value pairs where the keys are integers and the
values are integers and strings. The keys are encoded as a variable length
integer tag where the 3 lowest bits indicates the type of the value:
0 for integers, 2 for strings. If the value is an integer then the tag is
followed by the value encoded as a variable length integer. If the value is
a string then the tag is followed by the length of the string encoded as
a variable length integer followed by the string itself.

Olm uses a variable length encoding for integers. Each integer is encoded as a
sequence of bytes with the high bit set followed by a byte with the high bit
clear. The seven low bits of each byte store the bits of the integer. The least
significant bits are stored in the first byte.

============= ===== ======== ================================================
    Name       Tag    Type                     Meaning
============= ===== ======== ================================================
Message-Index  0x08 Integer  The index of the ratchet, :math:`i`
Cipher-Text    0x12 String   The cipher-text, :math:`X_{i}`, of the message
============= ===== ======== ================================================

The length of the MAC is determined by the authenticated encryption algorithm
being used (8 bytes in this version of the protocol). The MAC protects all of
the bytes preceding the MAC.

The length of the signature is determined by the signing algorithm being used
(64 bytes in this version of the protocol). The signature covers all of the
bytes preceding the signaure.

IPR
---

The Megolm specification (this document) is hereby placed in the public domain.

Feedback
--------

Can be sent to richard at matrix.org.


.. _`Ed25519`: http://ed25519.cr.yp.to/
.. _`HMAC-based key derivation function`: https://tools.ietf.org/html/rfc5869
.. _`HKDF-SHA-256`: https://tools.ietf.org/html/rfc5869
.. _`HMAC-SHA-256`: https://tools.ietf.org/html/rfc2104
.. _`SHA-256`: https://tools.ietf.org/html/rfc6234
.. _`AES-256`: http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
.. _`CBC`: http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf
.. _`PCKS#7`: https://tools.ietf.org/html/rfc2315
.. _`Olm`: ./olm.html
