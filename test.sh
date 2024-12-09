#!/bin/bash
set -eu
PATH="$(realpath "$(dirname "$0")/build"):$PATH"
cd /tmp
fuzz-salmagundi -max_len=65538 -jobs=$(nproc) -runs=9999
test-salmagundi
