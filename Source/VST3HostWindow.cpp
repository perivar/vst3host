#include "VST3HostWindow.h"

VST3HostWindow::VST3HostWindow()
    : DocumentWindow("VST3 Host",
                    juce::Colours::lightgrey,
                    juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setContentOwned(new MainComponent(), true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    setResizable(true, true);
}

void VST3HostWindow::closeButtonPressed()
{
    juce::JUCEApplication::quit();
}