@echo off
setlocal enabledelayedexpansion

echo Checking for vcpkg...
if not exist vcpkg (
    echo Cloning vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if !errorlevel! neq 0 (
        echo Error cloning vcpkg
        exit /b 1
    )
)

echo Bootstrapping vcpkg...
call vcpkg\bootstrap-vcpkg.bat
if %errorlevel% neq 0 (
    echo Error bootstrapping vcpkg
    exit /b 1
)

echo Installing JUCE...
vcpkg\vcpkg install juce:x64-windows
if %errorlevel% neq 0 (
    echo Error installing JUCE
    exit /b 1
)

echo Setup completed successfully!