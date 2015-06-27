Olm
===

An implementation of the axolotl ratchet as described by
https://github.com/trevp/axolotl/wiki, written in C++11 and exposed as a C API

Building
--------

To build olm as a shared library run:

.. code:: bash

    ./build_shared_library.py

To run the tests run:

.. code:: bash

   ./test.py


To build the javascript bindings, install emscripten from http://kripken.github.io/emscripten-site/ and then run:

.. code:: bash

    javascript/build.py

Design
------

Olm is designed to be easy port to different platforms and to be easy
to write bindings for.

Error Handling
~~~~~~~~~~~~~~

All C functions in the API for olm return ``olm_error()`` on error.
This makes it easy to check for error conditions within the language bindings.

Random Numbers
~~~~~~~~~~~~~~

Olm doesn't generate random numbers itself. Instead the caller must
provide the random data. This makes it easier to port the library to different
platforms since the caller can use whatever cryptographic random number
generator their platform provides.

Memory
~~~~~~

Olm avoids calling malloc or allocating memory on the heap itself.
Instead the library calculates how much memory will be needed to hold the
output and the caller supplies a buffer of the appropriate size.

Output Encoding
~~~~~~~~~~~~~~~

Binary output is encoded as base64 so that languages that prefer unicode
strings will find it easier to handle the output.

Dependencies
~~~~~~~~~~~~

Olm uses pure C implementations of the cryptographic primitives used by
the ratchet. While this decreases the performance it makes it much easier
to compile the library for different architectures.

What's an olm?
~~~~~~~~~~~~~~

It's a really cool species of European troglodytic salamander.
Matthew once tried to climb into a pool full of them in Postojnska Jama.
http://www.postojnska-jama.eu/en/about-the-cave/meet-the-dragon-s-offspring/
