#!/bin/sh

# List of FOREACH macroses

grep -RI -h '^#define [^[:space:]]*FOREACH[^[:space:]]*(' . | sed "s/^#define \([^[:space:]]*FOREACH[^[:space:]]*\)(.*$/  - '\1'/" | sort | uniq
