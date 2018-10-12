#!/usr/bin/env python

import sys
import re
import json

expr = re.compile(r"(olm_[^( ]*)\(")

exports = set()

for f in sys.argv[1:]:
    with open(f) as fp:
        for line in fp:
            matches = expr.search(line)
            if matches is not None:
                exports.add('_%s' % (matches.group(1),))

json.dump(sorted(exports), sys.stdout)
