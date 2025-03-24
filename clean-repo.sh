#!/bin/bash

echo "Cleaning repository..."

# Clean build directory
if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
else
    echo "No build directory found."
fi

# Clean vcpkg directory
if [ -d "vcpkg" ]; then
    echo "Removing vcpkg directory..."
    rm -rf vcpkg
else
    echo "No vcpkg directory found."
fi

echo "Repository cleaned successfully!"