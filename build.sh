#!/usr/bin/env bash
set -eu

cmake -S . -B build
cmake --build build
