#!/usr/bin/env python3

import os
import re
import fnmatch
import argparse
import subprocess
import enum
import itertools
from typing import Iterator
from dataclasses import dataclass, field

from tabulate import tabulate


class TColors:
    BLUE = "\033[94m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    GRAY = "\033[90m"
    CBLINK = "\033[5m"
    END = "\033[0m"

def apply_color(text: str, color: TColors) -> str:
    return color + text + TColors.END

@dataclass
class TestCase:
    class CaseStatus(enum.Enum):
        NONE = -1
        IGNORE = 0
        PASS = 1
        FAIL = 2

    line: int = 0
    name: str = ""
    status: CaseStatus = CaseStatus.NONE
    message: str = ""
    # (Line, Message)
    additional_info: [(int, str)] = field(default_factory=list)

def get_status_color(s: TestCase.CaseStatus) -> TColors:
    table = {
        TestCase.CaseStatus.NONE: None,
        TestCase.CaseStatus.IGNORE: TColors.BLUE,
        TestCase.CaseStatus.PASS: TColors.GREEN,
        TestCase.CaseStatus.FAIL: TColors.RED,
    }
    return table[s]

def get_status_text(s: TestCase.CaseStatus) -> str:
    table = {
        TestCase.CaseStatus.NONE: "None",
        TestCase.CaseStatus.IGNORE: "Ignored",
        TestCase.CaseStatus.PASS: "Passed",
        TestCase.CaseStatus.FAIL: "Failed",
    }
    return table[s]


@dataclass
class Test:
    class TestStatus(enum.Enum):
        OK = 1
        TERMINATED = 2
        UNKNOWN = 3

    status: TestStatus = TestStatus.UNKNOWN
    exec_path: str = ""
    source_file: str = ""
    cases: [TestCase] = field(default_factory=list)
    sanitizers_out: bytes = bytes()


ENTRY_RE = re.compile(r"^(.*):(\d+):(\w+):(\w+)(:\s*(.*))?$")



def find_executables(path: str, pattern: str) -> Iterator[str]:
    for root, _, files in os.walk(path):
        for f in files:
            path = os.path.join(root, f)
            match = fnmatch.fnmatch(f, pattern)
            executable = os.access(path, os.X_OK)
            if match and executable:
                yield path


def run_tests(execs: Iterator[str]) -> Iterator[Test]:
    def was_terminated(p: subprocess.CompletedProcess) -> bool:
        # Sanitizer will always return 1 when there are errors.
        return p.returncode != 0

    def get_source_file(unity_output: bytes) -> str:
        output = unity_output.decode("utf-8")
        for l in output.splitlines():
            parsed = ENTRY_RE.search(l)
            if parsed is not None:
                return parsed.group(1)

    def get_test_cases(stdout: bytes) -> Iterator[TestCase]:
        output = stdout.decode("utf-8")
        case = TestCase()
        for l in output.splitlines():
            unity_output = ENTRY_RE.search(l)
            if unity_output is not None:
                status_str = unity_output.group(4)
                if status_str == "INFO":
                    case.additional_info.append(
                        (unity_output.group(2), unity_output.group(6))
                    )
                else:
                    case.line = int(unity_output.group(2))
                    case.name = unity_output.group(3)
                    case.status = TestCase.CaseStatus[unity_output.group(4)]
                    case.message = unity_output.group(6)

                    yield case
                    case = TestCase()

    for e in execs:
        p = subprocess.run(e, capture_output=True)

        # Ugh..
        unity_output = p.stdout
        san_output = p.stderr

        test = Test()
        test.exec_path = e
        test.source_file = get_source_file(unity_output)

        if was_terminated(p):
            test.status = Test.TestStatus.TERMINATED
        else:
            test.status = Test.TestStatus.OK

        test.cases = list(get_test_cases(unity_output))
        test.sanitizers_out = san_output

        yield test

def get_test_file(t: Test):
    if t.source_file is not None:
        return os.path.relpath(t.source_file)
    else:
        return os.path.relpath(t.exec_path)

def print_test_cases(tests_it: Iterator[Test]):
    cases = []
    for test in tests_it:
        cases += list(map(lambda c: (test, c), test.cases))

    cases = sorted(cases, key=lambda c: (c[1].status.value, get_test_file(c[0])))
    columns = {
        "Status": map(lambda c: apply_color(
            get_status_text(c[1].status), get_status_color(c[1].status)
        ), cases),
        "Location": map(lambda c: get_test_file(c[0]) + ':' + str(c[1].line), cases),
        "Name": map(lambda c: c[1].name, cases),
        "Message": map(lambda c: c[1].message, cases)
    }

    print(tabulate(columns, headers="keys", tablefmt="github"))


def print_terminated(tests_it: Iterator[Test]):
    for test in tests_it:
        if test.status != Test.TestStatus.OK and len(test.cases) > 0:
            print("File", get_test_file(test))
            for case in test.cases:
                print("\tCase", case.name + ":" + str(case.line))
                for info in case.additional_info:
                    print("\t\tLine", info[0], "\tMessage:", info[1])

def print_sanitizer_output(tests_it: Iterator[Test]):
    for test in tests_it:
        if len(test.sanitizers_out) > 0:
            print("File", get_test_file(test))
            os.sys.stdout.buffer.write(test.sanitizers_out)
            print("\n")


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(description="Run project tests")
    a.add_argument("-d", dest="build_dir", help="Build directory", default="./build")
    a.add_argument("-p", dest="pattern", help="Filenames pattern", default="*_tests")

    return a.parse_args()


def main():
    args = parse_args()

    ASAN_OPTIONS = [
        ("color", "always"),
        ("detect_stack_use_after_return", "true"),
        ("strict_string_checks", "true"),
        ("detect_invalid_pointer_pairs", "2"),
    ]
    asan_options_val = ":".join(("=".join(p) for p in ASAN_OPTIONS))
    os.putenv("ASAN_OPTIONS", asan_options_val)

    tests = find_executables(args.build_dir + "/tests", args.pattern)
    results = run_tests(tests)

    its = itertools.tee(results, 3)
    print("Test Cases:")
    print_test_cases(its[0])
    print("\nAdditional messages from terminated tests:")
    print_terminated(its[1])
    print("\nOutput from sanitizers:")
    print_sanitizer_output(its[2])


main()
