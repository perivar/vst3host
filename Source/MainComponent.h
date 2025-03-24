#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent,
                     public juce::MidiKeyboardStateListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

private:
    void openPluginBrowser();
    void scanAndLoadPlugin(const juce::File& file);
    void loadPlugin(const juce::PluginDescription& description);

    juce::TextButton loadButton{"Load VST3 Plugin"};
    juce::AudioPluginFormatManager formatManager;
    juce::AudioProcessorGraph processorGraph;
    std::unique_ptr<juce::AudioProcessorEditor> pluginEditor;
    std::unique_ptr<juce::AudioPluginInstance> pluginToLoad;
    std::unique_ptr<juce::FileChooser> chooser;
    
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    juce::MidiMessageCollector midiCollector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};