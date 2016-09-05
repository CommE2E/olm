#!/bin/sh

set -e

make clean
make lib
make test
./python/test_olm.sh

. ~/.emsdk_set_env.sh
make js
npm pack javascript
