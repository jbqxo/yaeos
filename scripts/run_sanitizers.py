#!/usr/bin/env python3

import os
import argparse
import fnmatch
from typing import Iterator
from dataclasses import dataclass, field

TESTS_PREFIX = "tests_san_"
AVAIL_SANS = []


class TColors:
    SELECT = "\033[7m"
    END = "\033[0m"


@dataclass
class Test:
    relpath: str = ""
    sans_execs: dict = field(default_factory=dict)


def find_executables(path, pattern) -> Iterator[str]:
    for root, _, files in os.walk(path):
        for f in files:
            path = os.path.join(root, f)
            match = fnmatch.fnmatch(f, pattern)
            executable = os.access(path, os.X_OK)
            if match and executable:
                yield path


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(description="Run project sanitizers")
    a.add_argument("-d", dest="build_dir",
                   help="Build directory", default="./build")
    a.add_argument("-p", dest="pattern",
                   help="Filenames pattern", default="*_tests")

    return a.parse_args()


def find_avail_san_tests(build_dir) -> Iterator[str]:
    for entry in os.listdir(build_dir):
        if entry.startswith(TESTS_PREFIX):
            yield entry[len(TESTS_PREFIX):]


def run_and_print(tests):
    for test in tests:
        print((TColors.SELECT +
               "Running sanitizers for {0}" + TColors.END).format(test.relpath))
        for key, value in test.sans_execs.items():
            os.system(os.path.abspath(value))


def main():
    args = parse_args()

    for san in find_avail_san_tests(args.build_dir):
        AVAIL_SANS.append(san)

    tests_dir = args.build_dir + "/tests/"
    tests = []
    for executable in find_executables(tests_dir, "*_tests"):
        relpath = executable[len(tests_dir):]
        sans_execs = {}
        for san in AVAIL_SANS:
            sans_execs[san] = "/".join([args.build_dir,
                                        TESTS_PREFIX + san,
                                        relpath])

        tests.append(Test(relpath, sans_execs))

    run_and_print(tests)


main()
