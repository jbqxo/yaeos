#!/usr/bin/env python3

"""
The script extracts field offsets from a C file using a mighty clang
and generates a header with a bunch of preprocessor-defines out of it.
"""

from clang import cindex
from typing import Iterator
from dataclasses import dataclass
import itertools
import argparse
import re
import sys


@dataclass
class Definition:
    struct: str
    field: str
    offset: int


def find_exported_definitions(pattern: str, tu: cindex.TranslationUnit) -> Iterator[Definition]:
    patternobj = re.compile(pattern)

    structs = (node for node in tu.cursor.get_children()
               if node.kind == cindex.CursorKind.STRUCT_DECL)

    for s in structs:
        fields_with_comments = (field for field in s.get_children()
                                if field.brief_comment)
        exported_fields = (field for field in fields_with_comments
                           if patternobj.search(field.brief_comment))

        for field in exported_fields:
            yield Definition(s.spelling, field.spelling, field.get_field_offsetof())


def get_comp_args(target_file: str, compdb: cindex.CompilationDatabase) -> Iterator[str]:
    result = list()

    for db_cmd in compdb.getCompileCommands(target_file):
        result.extend(db_cmd.arguments)

    # Remove compiler.
    result = result[1:]

    # Remove filename itself.
    result = result[:len(result) - 1]

    # Parse all comments. Clang, by default, parses only doxygen.
    result.append("-fparse-all-comments")
    return result


def is_iterator_empty(it):
    # TODO: Is there a better way?
    try:
        first = next(it)
    except StopIteration:
        return True, None
    return False, itertools.chain([first], it)


def gen_offset_file(defs: [Definition], file_path: str) -> bool:
    # Do not create a file if there are no definitions to write.
    empty, defs = is_iterator_empty(defs)
    if empty:
        return False

    with open(file_path, "w") as f:
        for d in defs:
            print("#define OFFSETS__{0}__{1} {2}".format(
                d.struct.upper(), d.field.upper(), int(d.offset / 8)), file=f)

    return True


def parse_args() -> argparse.Namespace:
    a = argparse.ArgumentParser(
        description="Generate a header file with structs' offsets.")
    a.add_argument("-f", dest="file", help="File to handle.", required=True)
    a.add_argument("-c", dest="compdbdir",
                   help="Directory where a compilation database can be found.", default=".")
    a.add_argument("--libclang", dest="libclang",
                   help="Path to the clang's libraries.")
    a.add_argument("-p", dest="pattern", default="^\\s*ASM_EXPORT_OFFSET",
                   help="Regex expression to use for the search.")
    a.add_argument("-o", "--output", dest="output", required=True,
                   help="Where to write the header file.")

    return a.parse_args()


def main():
    args = parse_args()

    if args.libclang is not None:
        cindex.Config.set_library_path(args.libclang)

    compdb = cindex.CompilationDatabase.fromDirectory(args.compdbdir)
    comp_args = get_comp_args(args.file, compdb)
    index = cindex.Index.create()

    tu = index.parse(args.file, comp_args)
    definitions = find_exported_definitions(args.pattern, tu)
    file_created = gen_offset_file(definitions, args.output)

    if file_created:
        sys.exit(0)
    else:
        sys.exit(1)


main()
