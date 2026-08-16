#!/usr/bin/env bash
set -eu

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
