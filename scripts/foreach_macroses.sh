#!/bin/sh

# List of FOREACH macroses

git grep -h '^#define [^[:space:]]*FOREACH[^[:space:]]*(' HEAD  | sed "s,^#define \([^[:space:]]*FOREACH[^[:space:]]*\)(.*$,  - '\1'," | sort | uniq
