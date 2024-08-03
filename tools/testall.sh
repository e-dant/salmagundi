#! /usr/bin/env bash
set -e
cd "$(dirname "$0")/.."
tools/buildall.sh
find out -name 'host*' | while read h
do find "$h" -name test-salmagundi -exec {} \;
done

