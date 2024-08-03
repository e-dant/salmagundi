#! /usr/bin/env bash
set -e
cd "$(dirname "$0")/.."
[ -d out/host-asan ] || meson setup out/host-asan -Db_sanitize=address -Db_lundef=false
[ -d out/host-ubsan ] || meson setup out/host-ubsan -Db_sanitize=undefined -Db_lundef=false
[ -d out/host-msan ] || meson setup out/host-msan -Db_sanitize=memory -Db_lundef=false
[ -d out/host ] || meson setup out/host
meson compile -C out/host-asan || echo not supported for host
meson compile -C out/host-ubsan || echo not supported for host
meson compile -C out/host-msan || echo not supported for host
meson compile -C out/host
cp out/host/compile_commands.json .
