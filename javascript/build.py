#! /usr/bin/env python
# Copyright 2015 OpenMarket Ltd
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

import subprocess
import glob
import os
import sys
import re
import json

source_files = glob.glob("src/*.cpp")
pre_js, = glob.glob("javascript/*pre.js")
post_js, = glob.glob("javascript/*post.js")

if not os.path.exists("build"):
    os.mkdir("build")

functions = set()
RE_FUNCTION=re.compile("(olm_[^( ]*)\\(")
with open("include/olm/olm.hh") as header:
    for line in header:
        match = RE_FUNCTION.search(line)
        if match:
            functions.add(match.groups()[0])


exported_functions = os.path.abspath("build/exported_functions.json")
with open(exported_functions, "w") as json_file:
    json.dump(["_" + function for function in functions], json_file)


emcc = os.environ.get("EMCC", "emcc")

compile_args = [emcc]
compile_args += """
    -O3
    -Iinclude
    -Ilib
    -std=c++11
    --closure 1
    --memory-init-file 0
    -s NO_FILESYSTEM=1
    -s NO_BROWSER=1
    -s INVOKE_RUN=0
""".split()
compile_args += source_files
compile_args += ("--pre-js", pre_js)
compile_args += ("--post-js", post_js)
compile_args += ("-s", "EXPORTED_FUNCTIONS=@" + exported_functions)
compile_args += sys.argv[1:]

library = "build/olm.js"

def run(args):
    print args
    print " ".join(args)
    subprocess.check_call(args)

run(compile_args + ["-o", library])

