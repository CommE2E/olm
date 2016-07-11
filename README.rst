Olm
===

An implementation of the cryptographic ratchet described by
https://github.com/trevp/axolotl/wiki, written in C++11 and exposed as a C API

The specification of the Olm ratchet can be found in docs/olm.rst or
https://matrix.org/docs/spec/olm.html

Building
--------

To build olm as a shared library run:

.. code:: bash

    make

To run the tests run:

.. code:: bash

    make test

To build the javascript bindings, install emscripten from http://kripken.github.io/emscripten-site/ and then run:

.. code:: bash

    make js

Release process
---------------

# Bump version numbers in ``Makefile`` and ``javascript/package.json``
# Prepare changelog
# ``git commit``
# ``make test``
# ``make js``
# ``npm pack javascript``
# ``scp olm-x.y.z.tgz packages@ldc-prd-matrix-001:/sites/matrix/packages/npm/olm/``
# ``git tag x.y.z``
# ``git push --tags``

It's probably sensible to do the above on a release branch (``release-vx.y.z``
by convention), and merge back to master once complete.


Design
------

Olm is designed to be easy port to different platforms and to be easy
to write bindings for.

It was originally implemented in C++, with a plain-C layer providing the public
API. As development has progressed, it has become clear that C++ gives little
advantage, and new functionality is being added in C, with C++ parts being
rewritten as the need ariases.

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
--------------

It's a really cool species of European troglodytic salamander.
http://www.postojnska-jama.eu/en/come-and-visit-us/vivarium-proteus/

Legal Notice
------------

The software may be subject to the U.S. export control laws and regulations
and by downloading the software the user certifies that he/she/it is
authorized to do so in accordance with those export control laws and
regulations.
