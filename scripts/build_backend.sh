#!/bin/bash
# Script to build Fast DDS Backend on WSL2

echo "[INFO] Building Fast DDS Backend..."
cd "/mnt/c/Project TTS/dds_demo/wsl_dashboard/backend"

# Ensure build directory exists
mkdir -p build
cd build

# Run CMake and Make
cmake ..
make -j4

echo "[INFO] Build complete! Executable is at wsl_dashboard/backend/build/subscriber"
