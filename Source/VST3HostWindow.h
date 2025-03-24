#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"

class VST3HostWindow : public juce::DocumentWindow
{
public:
    VST3HostWindow();
    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST3HostWindow)
};