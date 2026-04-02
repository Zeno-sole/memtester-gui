#!/bin/bash
set -e

echo "Building memtester-gui Debian package..."

# Check if we're in the right directory
if [ ! -f "debian/control" ]; then
    echo "Error: Must run from memtester-gui directory"
    exit 1
fi

# Install build dependencies
echo "Installing build dependencies..."
sudo apt-get update
sudo apt-get install -y debhelper cmake qt6-base-dev libqt6widgets6 \
    libqt6gui6 libqt6core6 dpkg-dev

# Clean previous builds
rm -rf debian/memtester-gui debian/.debhelper debian/files debian/*.debhelper.log \
       debian/*.substvars debian/debhelper-build-stamp

# Build the package
echo "Building package..."
dpkg-buildpackage -us -uc -b

echo "Package built successfully!"
echo "Check ../*.deb for the package file"
