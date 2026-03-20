#!/bin/bash
# Compila el backend de ExtreamFS. Requiere: cmake, g++/build-essential.
set -e
cd "$(dirname "$0")/.."
mkdir -p build
cd build
cmake ..
make -j$(nproc 2>/dev/null || echo 2)
echo "Compilado: ./build/extreamfs"
