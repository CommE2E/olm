#! /usr/bin/python
import subprocess
import glob
import os

if not os.path.exists("build"):
    os.mkdir("build")

test_files = glob.glob("tests/test_*.cpp")
source_files = glob.glob("src/*.cpp")

compile_args = "g++ -Itests/include -Iinclude -Ilib --std=c++11".split()
compile_args += source_files

for test_file in test_files:
    exe_file = "build/" + test_file[:4]
    subprocess.check_call(compile_args + [test_file, "-o", exe_file])
    subprocess.check_call([exe_file])
