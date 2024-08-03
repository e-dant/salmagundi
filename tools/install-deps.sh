#! /usr/bin/env bash
[ -f "$(dirname "$0")/../include/rapidhash.h" ] || wget -q https://github.com/Nicoshev/rapidhash/raw/master/rapidhash.h -O "$(dirname "$0")/../include/rapidhash.h"
