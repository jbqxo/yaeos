#!/usr/bin/env python3

import os
import sys
import fnmatch
import argparse
import subprocess
from typing import Iterator


def find_executables(path: str, pattern: str) -> Iterator[str]:
    for root, _, files in os.walk(path):
        for f in files:
            path = os.path.join(root, f)
            match = fnmatch.fnmatch(f, pattern)
            executable = os.access(path, os.X_OK)
            if match and executable:
                yield path


def run_execs(execs: Iterator[str]):
    for e in execs:
        p = subprocess.run(e, capture_output=True)
        print("Running {}".format(e), flush=True)
        sys.stdout.buffer.write(p.stdout)
        print("\n", flush=True)


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(description="Run project tests")
    a.add_argument("-d", dest="dir", help="Directory that contains tests", default="./build")
    a.add_argument("-p", dest="pattern", help="Filenames pattern", default="*_tests")

    return a.parse_args()


def main():
    args = parse_args()
    run_execs(find_executables(args.dir, args.pattern))


main()
