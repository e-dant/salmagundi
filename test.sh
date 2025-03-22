#!/bin/bash
set -eux
cd "$(dirname "$0")"
test -d fuzz-corpus || mkdir fuzz-corpus
build/fuzz-salmagundi -max_len=65538 -runs=10000 fuzz-corpus
build/test-salmagundi
