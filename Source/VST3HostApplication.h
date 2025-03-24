#pragma once

#include <JuceHeader.h>
#include "VST3HostWindow.h"

class VST3HostApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    void initialise(const juce::String& commandLine) override;
    void shutdown() override;

private:
    std::unique_ptr<VST3HostWindow> mainWindow;
};