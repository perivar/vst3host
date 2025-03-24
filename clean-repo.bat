@echo off
echo Cleaning repository...

:: Clean build directory
if exist build (
    echo Removing build directory...
    rmdir /s /q build
) else (
    echo No build directory found.
)

:: Clean vcpkg directory
if exist vcpkg (
    echo Removing vcpkg directory...
    rmdir /s /q vcpkg
) else (
    echo No vcpkg directory found.
)

echo Repository cleaned successfully!