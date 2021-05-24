# Olm

An implementation of the Double Ratchet cryptographic ratchet described by
https://whispersystems.org/docs/specifications/doubleratchet/, written in C and
C++11 and exposed as a C API.

The specification of the Olm ratchet can be found in [docs/olm.md](docs/olm.md).

This library also includes an implementation of the Megolm cryptographic
ratchet, as specified in [docs/megolm.md](docs/megolm.md).

## Building

To build olm as a shared library run either:

```bash
cmake . -Bbuild
cmake --build build
```

or:

```bash
make
```

Using cmake is the preferred method for building the shared library; the
Makefile may be removed in the future.

To run the tests when using cmake, run:

```bash
cd build/tests
ctest .
```
To run the tests when using make, run:

```bash
make test
```

To build the JavaScript bindings, install emscripten from http://kripken.github.io/emscripten-site/ and then run:

```bash
make js
```

Note that if you run emscripten in a docker container, you need to pass through
the EMCC_CLOSURE_ARGS environment variable.

To build the android project for Android bindings, run:

```bash
cd android
./gradlew clean assembleRelease
```

To build the Xcode workspace for Objective-C bindings, run:

```bash
cd xcode
pod install
open OLMKit.xcworkspace
```

To build the Python bindings, first build olm as a shared library as above, and
then run:

```bash
cd python
make
```

to make both the Python 2 and Python 3 bindings.  To make only one version, use
``make olm-python2`` or ``make olm-python3`` instead of just ``make``.

To build olm as a static library (which still needs libstdc++ dynamically) run
either:

```bash
cmake . -Bbuild -DBUILD_SHARED_LIBS=NO
cmake --build build
```

or

```bash
make static
```

The library can also be used as a dependency with CMake using:

```cmake
find_package(Olm::Olm REQUIRED)
target_link_libraries(my_exe Olm::Olm)
```

## Bindings

libolm can be used in different environments using bindings. In addition to the
JavaScript, Python, Java (Android), and Objective-C bindings included in this
repository, some bindings are (in alphabetical order):

- [cl-megolm](https://github.com/K1D77A/cl-megolm) (MIT) Common Lisp bindings
- [dart-olm](https://gitlab.com/famedly/libraries/dart-olm) (AGPLv3) Dart bindings
- [Dhole/go-olm](https://github.com/Dhole/go-olm) (Apache-2.0) Go bindings
- [libQtOlm](https://gitlab.com/b0/libqtolm/) (GPLv3) Qt bindings
- [matrix-kt](https://github.com/Dominaezzz/matrix-kt) (Apache-2.0) Kotlin
  library for Matrix, including Olm methods
- [maunium.net/go/mautrix/crypto/olm](https://github.com/tulir/mautrix-go/tree/master/crypto/olm)
  (Apache-2.0) fork of Dhole/go-olm
- [nim-olm](https://gitea.com/BarrOff/nim-olm) (MIT) Nim bindings
- [olm-sys](https://gitlab.gnome.org/BrainBlasted/olm-sys) (Apache-2.0) Rust
  bindings

Note that bindings may have a different license from libolm, and are *not*
endorsed by the Matrix.org Foundation C.I.C.

## Release process

First: bump version numbers in ``common.mk``, ``CMakeLists.txt``,
``javascript/package.json``, ``python/olm/__version__.py``, ``OLMKit.podspec``, ``Package.swift``,
and ``android/olm-sdk/src/main/java/org/matrix/olm/OlmManager.java`` in function ``getVersion()```.

Also, ensure the changelog is up to date, and that everything is committed to
git.

It's probably sensible to do the above on a release branch (``release-vx.y.z``
by convention), and merge back to master once the release is complete.

```bash
make clean

# build and test C library
make test

# build and test JS wrapper
make js
(cd javascript && \
     npm run test && \
     sha256sum olm.js olm_legacy.js olm.wasm > checksums.txt && \
     gpg -b -a -u F75FDC22C1DE8453 checksums.txt && \
     npm publish)

VERSION=x.y.z
git tag $VERSION -s
git push --tags

# OLMKit CocoaPod release
# Make sure the version OLMKit.podspec is the same as the git tag
# (this must be checked before git tagging)
pod spec lint OLMKit.podspec --use-libraries --allow-warnings
pod trunk push OLMKit.podspec --use-libraries --allow-warnings
# Check the pod has been successully published with:
pod search OLMKit
```

Python and JavaScript packages are published to the registry at
<https://gitlab.matrix.org/matrix-org/olm/-/packages>.  The GitLab
documentation contains instructions on how to set up twine (Python) and npm
(JavaScript) to upload to the registry.


## Design

Olm is designed to be easy port to different platforms and to be easy
to write bindings for.

It was originally implemented in C++, with a plain-C layer providing the public
API. As development has progressed, it has become clear that C++ gives little
advantage, and new functionality is being added in C, with C++ parts being
rewritten as the need ariases.

### Error Handling

All C functions in the API for olm return ``olm_error()`` on error.
This makes it easy to check for error conditions within the language bindings.

### Random Numbers

Olm doesn't generate random numbers itself. Instead the caller must
provide the random data. This makes it easier to port the library to different
platforms since the caller can use whatever cryptographic random number
generator their platform provides.

### Memory

Olm avoids calling malloc or allocating memory on the heap itself.
Instead the library calculates how much memory will be needed to hold the
output and the caller supplies a buffer of the appropriate size.

### Output Encoding

Binary output is encoded as base64 so that languages that prefer unicode
strings will find it easier to handle the output.

### Dependencies

Olm uses pure C implementations of the cryptographic primitives used by
the ratchet. While this decreases the performance it makes it much easier
to compile the library for different architectures.

## Contributing

Please see [CONTRIBUTING.md](CONTRIBUTING.md) when making contributions to the library.

## Security assessment

Olm 1.3.0 was independently assessed by NCC Group's Cryptography Services
Practive in September 2016 to check for security issues: you can read all
about it at
https://www.nccgroup.com/globalassets/our-research/us/public-reports/2016/november/ncc_group_olm_cryptogrpahic_review_2016_11_01.pdf
and https://matrix.org/blog/2016/11/21/matrixs-olm-end-to-end-encryption-security-assessment-released-and-implemented-cross-platform-on-riot-at-last/

## Bug reports

Please file bug reports at https://github.com/matrix-org/olm/issues

## What's an olm?

It's a really cool species of European troglodytic salamander.
http://www.postojnska-jama.eu/en/come-and-visit-us/vivarium-proteus/

## Legal Notice

The software may be subject to the U.S. export control laws and regulations
and by downloading the software the user certifies that he/she/it is
authorized to do so in accordance with those export control laws and
regulations.
