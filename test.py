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

if not os.path.exists("build"):
    os.mkdir("build")

test_files = glob.glob("tests/test_*.cpp")

compile_args = ("g++ -g -O0 -Itests/include -Iinclude -Ilib --std=c++11 "+
                "-L build").split()

def run(args, *xargs, **kwargs):
    print " ".join(args)
    subprocess.check_call(args, *xargs, **kwargs)

for test_file in test_files:
    exe_file = "build/" + test_file[6:-4]
    run(compile_args + [test_file, "-lolm", "-o", exe_file])
    run([exe_file], env={'LD_LIBRARY_PATH':'./build'})
