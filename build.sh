#!/bin/bash
set -euo pipefail
export CC=clang
[ -d build ] || meson setup build
ninja -C build
