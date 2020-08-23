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


class TColors:
    BLUE = "\033[94m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    GRAY = "\033[90m"
    CBLINK = "\033[5m"
    END = "\033[0m"


@dataclass
class TestCase:
    class Status(enum.Enum):
        NONE = -1
        IGNORE = 0
        PASS = 1
        FAIL = 2
        ERROR = 3

    exec_path: str = ""
    source_file: str = ""
    line: int = 0
    name: str = ""
    status: Status = Status.NONE
    message: str = ""
    # (Location, Message)
    additional_info: [(str, str)] = field(default_factory=list)


def find_executables(path: str, pattern: str) -> Iterator[str]:
    for root, _, files in os.walk(path):
        for f in files:
            path = os.path.join(root, f)
            match = fnmatch.fnmatch(f, pattern)
            executable = os.access(path, os.X_OK)
            if match and executable:
                yield path


def parse_output(exec: str, stdout: bytes) -> Iterator[TestCase]:

    output = stdout.decode("utf-8")
    unity_re = re.compile(r"^(.*):(\d+):(\w+):(\w+)(:\s*(.*))?$")
    case = TestCase()
    for l in output.splitlines():
        unity_output = unity_re.search(l)
        if unity_output is not None:
            status_str = unity_output.group(4)
            if status_str == "INFO":
                filename = os.path.basename(unity_output.group(1))
                case.additional_info.append(
                    (":".join([filename, unity_output.group(2)]),
                     unity_output.group(6)))
            else:
                case.exec_path = exec
                case.source_file = unity_output.group(1)
                case.line = int(unity_output.group(2))
                case.name = unity_output.group(3)
                case.status = TestCase.Status[unity_output.group(4)]
                case.message = unity_output.group(6)

                yield case
                case = TestCase()


def run_tests(execs: Iterator[str]) -> Iterator[TestCase]:
    for e in execs:
        p = subprocess.run(e, capture_output=True)
        if p.returncode != 0:
            status = TestCase.Status.ERROR
            message = "Return code is {0}".format(p.returncode)
            yield TestCase(exec_path=e, status=status, message=message)
        else:
            for case in parse_output(e, p.stdout):
                yield case


def get_col_len(column):
    return max(map(len, column), default=0)


def present_unit_results(cases_it):
    its = itertools.tee(cases_it, 4)
    columns = [map(lambda c: str(c.status), its[0]),
               map(lambda c: c.exec_path + str(c.line), its[1]),
               map(lambda c: c.name, its[2])]
    columns_len = list(map(get_col_len, columns))

    for case in sorted(its[3], key=lambda k: (k.status.value, k.exec_path)):
        status = ""
        if case.status == TestCase.Status.IGNORE:
            status = TColors.BLUE + "IGNORE" + TColors.END
        elif case.status == TestCase.Status.FAIL:
            status = TColors.RED + "FAIL" + TColors.END
        elif case.status == TestCase.Status.PASS:
            status = TColors.GREEN + "PASS" + TColors.END
        elif case.status == TestCase.Status.ERROR:
            status = TColors.RED + TColors.CBLINK + "ERROR" + TColors.END + TColors.END

        message = ""
        if case.message is not None:
            message = case.message

        print("{0}\t{1}\t{2}\t{3}".format(
            status.ljust(columns_len[0]),
            ":".join([case.exec_path, str(case.line)]).ljust(columns_len[1]),
            case.name.ljust(columns_len[2]),
            message))

        col_len0 = get_col_len(map(lambda i: i[0], case.additional_info))
        col_len1 = get_col_len(map(lambda i: i[1], case.additional_info))
        for info in case.additional_info:
            print((TColors.GRAY + "\t{0}\t{1}" + TColors.END).format(
                info[0].ljust(col_len0),
                info[1].ljust(col_len1)))


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(description="Run project tests")
    a.add_argument("-d", dest="build_dir",
                   help="Build directory", default="./build")
    a.add_argument("-p", dest="pattern",
                   help="Filenames pattern", default="*_tests")

    return a.parse_args()


def main():
    args = parse_args()
    tests = find_executables(args.build_dir + "/tests", args.pattern)
    cases = run_tests(tests)
    present_unit_results(cases)


main()
