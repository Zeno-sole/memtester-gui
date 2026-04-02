#!/bin/bash

# Memtester GUI Build Script

set -e

echo "Building Memtester GUI..."

# Check for Qt6
if ! pkg-config --exists Qt6Core Qt6Gui Qt6Widgets; then
    echo "Error: Qt6 development packages not found"
    echo "Please install Qt6 development packages:"
    echo "  Ubuntu/Debian: sudo apt install qt6-base-dev"
    echo "  Fedora: sudo dnf install qt6-qtbase-devel"
    echo "  Arch: sudo pacman -S qt6-base"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "Build complete! Run with: ./build/memtester-gui"
