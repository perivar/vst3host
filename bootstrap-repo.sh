#!/bin/bash

echo "Checking for vcpkg..."
if [ ! -d "vcpkg" ]; then
    echo "Cloning vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git
    if [ $? -ne 0 ]; then
        echo "Error cloning vcpkg"
        exit 1
    fi
fi

echo "Bootstrapping vcpkg..."
./vcpkg/bootstrap-vcpkg.sh
if [ $? -ne 0 ]; then
    echo "Error bootstrapping vcpkg"
    exit 1
fi

echo "Installing JUCE..."
# Determine the triplet based on the OS
if [ "$(uname)" == "Darwin" ]; then
    TRIPLET="x64-osx"
else
    TRIPLET="x64-linux"
fi

./vcpkg/vcpkg install juce:$TRIPLET
if [ $? -ne 0 ]; then
    echo "Error installing JUCE"
    exit 1
fi

echo "Setup completed successfully!"