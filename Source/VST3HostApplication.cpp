#include "VST3HostApplication.h"

const juce::String VST3HostApplication::getApplicationName()
{
    return "VST3Host";
}

const juce::String VST3HostApplication::getApplicationVersion()
{
    return "1.0.0";
}

void VST3HostApplication::initialise(const juce::String& /*commandLine*/)
{
    mainWindow = std::make_unique<VST3HostWindow>();
}

void VST3HostApplication::shutdown()
{
    mainWindow = nullptr;
}