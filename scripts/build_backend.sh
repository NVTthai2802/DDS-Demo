#!/bin/bash
set -e

# Xác định đường dẫn gốc của dự án một cách tương đối
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Building Backend (C++) in $PROJECT_ROOT/backend"

cd "$PROJECT_ROOT/backend"
mkdir -p build
cd build

# CMake generate
cmake ..

# Build
make -j$(nproc)

echo "Build successful! Executable is located at: $PROJECT_ROOT/backend/build/backend_node"