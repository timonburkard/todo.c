$ErrorActionPreference = "Stop"

cmake -S . -B build
cmake --build build
