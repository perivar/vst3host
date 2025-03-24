cmake_minimum_required(VERSION 3.15)
project(VST3Host VERSION 1.0.0)

# Point to vcpkg toolchain file
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
    set(JUCE_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/share/juce")
else()
    set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "")
    set(JUCE_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/arm64-osx/share/juce")
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
set(CMAKE_PREFIX_PATH ${JUCE_CONFIG_PATH}
    CACHE STRING "Path to JUCE config")

# Find JUCE
find_package(JUCE CONFIG REQUIRED)

# Define the application
juce_add_gui_app(VST3Host
    PRODUCT_NAME "VST3 Host"
)

# Enable VST3 host support
target_compile_definitions(VST3Host
    PUBLIC
        JUCE_PLUGINHOST_VST3=1
)

# Link JUCE modules (must come before juce_generate_juce_header)
target_link_libraries(VST3Host
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(MSVC)
    target_compile_options(VST3Host PRIVATE 
        /W4     # Warning level 4
        /MP     # Multi-processor compilation
    )
    
    # Enable parallel builds
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VST3Host)
endif()

# Generate JuceHeader.h (after target is defined and linked)
juce_generate_juce_header(VST3Host)

# Add source files
target_sources(VST3Host
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/VST3HostApplication.cpp
        Source/VST3HostWindow.cpp
)