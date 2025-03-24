# VST3 Host

A VST3 host application built using [JUCE](https://juce.com/) and [vcpkg](https://vcpkg.io/) package manager.

## Setup Instructions

1. Clone this repository
2. Run the appropriate bootstrap script for your platform:
   - For macOS/Linux: `./bootstrap-repo.sh`
   - For Windows: `bootstrap-repo.bat`

The bootstrap script will:

- Clone vcpkg if not present
- Bootstrap vcpkg
- Install JUCE using vcpkg with the appropriate platform settings

## Building with CMake

When configuring the project in Visual Studio Code:

1. Open the Command Palette (Ctrl+Shift+P / Cmd+Shift+P)
2. Run "CMake: Configure"
3. Choose "Unspecified" when prompted for kit selection
   - This allows CMake to pick the default compiler which is optimized for your system

This ensures the best compiler selection for your specific platform and environment.

## Building the Application

1. Open the Command Palette (Ctrl+Shift+P / Cmd+Shift+P)
2. Run "CMake: Build"
3. The application will be built in the `build` directory