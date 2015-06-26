Axolotlpp
=========

An implementation of the axolotl ratchet written in C++11 and exposed as a C
API.

Building
--------

To build axolotlpp as a shared library run:

.. code:: bash

    ./build_shared_library.py

To run the tests run:

.. code:: bash

   ./test.py


To build the javascript bindings run:

.. code:: bash

    javascript/build.py

Design
------

Axolotlpp is designed to be easy port to different platforms and to be easy
to write bindings for.

Error Handling
~~~~~~~~~~~~~~

All C functions in the API for axolotlpp return ``axolotl_error()`` on error.
This makes it easy to check for error conditions within the language bindings.

Random Numbers
~~~~~~~~~~~~~~

Axolotlpp doesn't generate random numbers itself. Instead the caller must
provide the random data. This makes it easier to port the library to different
platforms since the caller can use whatever cryptographic random number
generator their platform provides.

Memory
~~~~~~

Axolotlpp avoids calling malloc or allocating memory on the heap itself.
Instead the library calculates how much memory will be needed to hold the
output and the caller supplies a buffer of the appropriate size.

Output Encoding
~~~~~~~~~~~~~~~

Binary output is encoded as base64 so that languages that prefer unicode
strings will find it easier to handle the output.

Dependencies
~~~~~~~~~~~~

Axolotlpp uses pure C implementations of the cryptographic primitives used by
the ratchet. While this decreases the performance it makes it much easier
to compile the library for different architectures.
