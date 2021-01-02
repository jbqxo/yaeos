#!/usr/bin/env python3

import json
import argparse
import fnmatch
import os
from typing import Iterator


def find_entries(path: str, pattern: str) -> Iterator[str]:
    for root, _, files in os.walk(path):
        for f in files:
            if fnmatch.fnmatch(f, pattern):
                yield os.path.join(root, f)


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(description="Merges compilation database entries")

    a.add_argument("-o", dest="output", help="Output file", default="compile_commands.json")
    a.add_argument("-d", dest="dir", help="Directory that contains cdb entries", default="./build")
    a.add_argument("-p", dest="pattern", help="Cdb entry filename pattern", default="*.json")
    a.add_argument("-c", dest="command_style", help="Build command-styled database", action="store_true")
    a.add_argument("-C", dest="new_compiler", help="Change compiler", default=None)
    a.add_argument("-e", dest="exclude", help="Space separated parameters to exclude", nargs=argparse.REMAINDER)
    return a.parse_args()


def transform_to_cmd(db: dict):
    db["command"] = db.pop("arguments")
    db["command"] = " ".join(db["command"])


def patch_compiler(db: dict, new_compiler: str):
    db["arguments"][0] = new_compiler


def exclude_args(db: dict, to_exclude: [str]):
    db["arguments"] = [a for a in db["arguments"] if a not in to_exclude]

def main():
    args = parse_args()
    compilation_db = []
    for entry_path in find_entries(args.dir, args.pattern):
        with open(entry_path, "r") as entry:
            e = json.loads(entry.read().rstrip().rstrip(','))
            if args.new_compiler is not None:
                patch_compiler(e, args.new_compiler)
            if args.command_style:
                transform_to_cmd(e)
            if len(args.exclude) > 0:
                exclude_args(e, args.exclude)
            compilation_db.append(e)

    with open(args.output, "w+") as outf:
        json.dump(compilation_db, outf, indent=4)


main()
