#!/bin/sh

set -e

make clean
make lib
make test

./python/test_olm.sh
