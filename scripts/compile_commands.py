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
    return a.parse_args()


def transform_to_cmd(db: dict) -> dict:
    db["command"] = db.pop("arguments")
    db["command"] = " ".join(db["command"])
    return db


def main():
    args = parse_args()
    compilation_db = []
    for entry_path in find_entries(args.dir, args.pattern):
        with open(entry_path, "r") as entry:
            e = json.loads(entry.read().rstrip().rstrip(','))
            if args.command_style:
                e = transform_to_cmd(e)
            compilation_db.append(e)

    with open(args.output, "w+") as outf:
        json.dump(compilation_db, outf, indent=4)


main()
