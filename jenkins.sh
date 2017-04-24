#!/bin/sh

set -e

make clean
rm -f olm-*.tgz

make lib
make test

./python/test_olm.sh
pep8 -v python

. ~/.emsdk_set_env.sh
make js
(cd javascript && npm install && npm run test)
npm pack javascript
