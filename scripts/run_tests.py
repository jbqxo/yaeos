#!/usr/bin/env python3

import os
import re
import fnmatch
import argparse
import subprocess
import enum
from typing import Iterator
from dataclasses import dataclass


class TColors:
    BLUE = "\033[94m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    END = "\033[0m"


@dataclass
class TestCase:
    class Status(enum.Enum):
        IGNORE = 0
        PASS = 1
        FAIL = 2

    exec_path: str
    source_file: str
    line: int
    name: str
    status: Status
    message: str


def find_executables(path: str, pattern: str) -> Iterator[str]:
    for root, _, files in os.walk(path):
        for f in files:
            path = os.path.join(root, f)
            match = fnmatch.fnmatch(f, pattern)
            executable = os.access(path, os.X_OK)
            if match and executable:
                yield path


def parse_output(exec: str, stdout: bytes) -> Iterator[TestCase]:
    lines = stdout.decode("utf-8").splitlines()
    re_obj = re.compile(r"^(.*):(\d+):(\w+):(\w+)(:\s*(.*))?$")
    for l in lines:
        m = re_obj.search(l)
        if m is not None:
            case = TestCase(exec_path=exec,
                            source_file=m.group(1),
                            line=int(m.group(2)),
                            name=m.group(3),
                            status=TestCase.Status[m.group(4)],
                            message=m.group(6))
            yield case


def run_tests(execs: Iterator[str]) -> Iterator[TestCase]:
    for e in execs:
        p = subprocess.run(e, capture_output=True)
        for case in parse_output(e, p.stdout):
            yield case


def present_results(cases: Iterator[TestCase]):
    for case in sorted(cases, key=lambda k: (k.status.value, k.source_file)):
        status = ""
        if case.status == TestCase.Status.IGNORE:
            status = TColors.BLUE + "IGNORE" + TColors.END
        elif case.status == TestCase.Status.FAIL:
            status = TColors.RED + "FAIL" + TColors.END
        elif case.status == TestCase.Status.PASS:
            status = TColors.GREEN + "PASS" + TColors.END

        message = ""
        if case.message is not None and len(case.message) != 0:
            message = "\tMessage: " + case.message

        print(f"{status}\t{case.exec_path}:{case.line}\t{case.name}{message}")


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(description="Run project tests")
    a.add_argument("-d", dest="dir",
                   help="Directory that contains tests", default="./build")
    a.add_argument("-p", dest="pattern",
                   help="Filenames pattern", default="*_tests")

    return a.parse_args()


def main():
    args = parse_args()
    tests = find_executables(args.dir, args.pattern)
    cases = run_tests(tests)
    present_results(cases)


main()
