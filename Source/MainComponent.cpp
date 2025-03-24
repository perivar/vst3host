#include "MainComponent.h"

MainComponent::MainComponent()
    : keyboardState(),
      keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    formatManager.addDefaultFormats();
    setAudioChannels(0, 2); // No input channels, stereo output
    
    addAndMakeVisible(keyboardComponent);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(loadPresetButton);
    addAndMakeVisible(savePresetButton);
    
    keyboardState.addListener(this);
    
    loadPresetButton.onClick = [this] { loadPreset(); };
    savePresetButton.onClick = [this] { savePreset(); };
    
    loadButton.onClick = [this] { openPluginBrowser(); };

    setSize(800, 600);
}

MainComponent::~MainComponent()
{
    keyboardState.removeListener(this);
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Configure the processor graph
    processorGraph.setPlayConfigDetails(0, 2, sampleRate, samplesPerBlockExpected);
    processorGraph.prepareToPlay(sampleRate, samplesPerBlockExpected);

    // Initialize MIDI collector
    midiCollector.reset(sampleRate);
}

void MainComponent::releaseResources()
{
    processorGraph.releaseResources();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Create a MIDI buffer for this block
    juce::MidiBuffer incomingMidi;

    // Get messages from the collector
    const auto timestamp = juce::Time::getMillisecondCounterHiRes();
    midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
    
    // Get messages from the keyboard state
    juce::MidiBuffer keyboardMidi;
    keyboardState.processNextMidiBuffer(keyboardMidi, 0, bufferToFill.numSamples, true);

    // Merge the keyboard MIDI into the incoming MIDI buffer
    for (const auto metadata : keyboardMidi)
    {
        auto samplePosition = static_cast<int>((metadata.getMessage().getTimeStamp() - timestamp) * 
                                             processorGraph.getSampleRate() / 1000.0);
        samplePosition = std::max(0, std::min(samplePosition, bufferToFill.numSamples - 1));
        incomingMidi.addEvent(metadata.getMessage(), samplePosition);
    }
    
    processorGraph.processBlock(*bufferToFill.buffer, incomingMidi);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    auto topBar = area.removeFromTop(30);
    
    loadButton.setBounds(topBar.removeFromLeft(150).reduced(2));
    auto presetButtonWidth = 100;
    loadPresetButton.setBounds(topBar.removeFromLeft(presetButtonWidth).reduced(2));
    savePresetButton.setBounds(topBar.removeFromLeft(presetButtonWidth).reduced(2));
    
    loadPresetButton.setEnabled(pluginEditor != nullptr);
    savePresetButton.setEnabled(pluginEditor != nullptr);
    
    if (pluginEditor != nullptr) {
        pluginEditor->setBounds(area.removeFromTop(area.getHeight() - 100).reduced(8));
    }
    
    keyboardComponent.setBounds(area.removeFromBottom(100).reduced(8));
}

void MainComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    auto message = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    midiCollector.addMessageToQueue(message);
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/)
{
    auto message = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
    message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    midiCollector.addMessageToQueue(message);
}

void MainComponent::openPluginBrowser()
{
    chooser = std::make_unique<juce::FileChooser>("Select a VST3 Plugin", 
        juce::File{}, "*.vst3");

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.exists())
            {
                scanAndLoadPlugin(result);
            }
        });
}

void MainComponent::scanAndLoadPlugin(const juce::File& file)
{
    if (formatManager.getFormats().isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Plugin Load Error",
            "No plugin formats available. Make sure VST3 module is properly initialized.");
        return;
    }

    juce::OwnedArray<juce::PluginDescription> descriptions;
    for (auto* format : formatManager.getFormats())
    {
        if (format->getName() == "VST3")
        {
            format->findAllTypesForFile(descriptions, file.getFullPathName());
            break;
        }
    }

    if (descriptions.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Plugin Load Error",
            "No valid VST3 plugin found in file.");
        return;
    }

    loadPlugin(*descriptions[0]);
}

void MainComponent::loadPlugin(const juce::PluginDescription& description)
{
    formatManager.createPluginInstanceAsync(description,
        processorGraph.getSampleRate(),
        processorGraph.getBlockSize(),
        [this](std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error)
        {
            if (instance == nullptr)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Plugin Load Error",
                    "Failed to load plugin: " + error);
                return;
            }

            pluginToLoad = std::move(instance);

            juce::MessageManager::callAsync([this]()
            {
                // Clear existing graph
                processorGraph.clear();

                if (!pluginToLoad)
                    return;

                // Create and add nodes
                auto* midiInputNode = processorGraph.addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
                    juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode)).get();

                auto* audioOutputNode = processorGraph.addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
                    juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode)).get();

                auto* pluginNode = processorGraph.addNode(std::move(pluginToLoad)).get();

                // Connect MIDI input to plugin
                processorGraph.addConnection({
                    { midiInputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex },
                    { pluginNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex }
                });

                // Connect plugin audio output to main output
                for (int channel = 0; channel < 2; ++channel)
                {
                    processorGraph.addConnection({
                        { pluginNode->nodeID, channel },
                        { audioOutputNode->nodeID, channel }
                    });
                }

                // Create and show editor
                pluginEditor.reset(pluginNode->getProcessor()->createEditorIfNeeded());
                if (pluginEditor != nullptr)
                {
                    addAndMakeVisible(pluginEditor.get());
                    resized();
                }
            });
        });
}

void MainComponent::loadPreset()
{
    if (auto* processor = getActiveProcessor())
    {
        chooser = std::make_unique<juce::FileChooser>(
            "Load Preset",
            juce::File{},
            "*.vstpreset;*.preset"
        );

        chooser->launchAsync(juce::FileBrowserComponent::openMode |
                           juce::FileBrowserComponent::canSelectFiles,
            [this, processor](const juce::FileChooser& fc)
            {
                const auto file = fc.getResult();
                if (file.exists())
                {
                    juce::MemoryBlock data;
                    if (!file.loadFileAsData(data))
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                            "Load Error", "Failed to read preset file: " + file.getFullPathName());
                        return;
                    }
                    
                    // Load the preset data
                    processor->setStateInformation(data.getData(), static_cast<int>(data.getSize()));
                }
            });
    }
}

void MainComponent::savePreset()
{
    if (auto* processor = getActiveProcessor())
    {
        chooser = std::make_unique<juce::FileChooser>(
            "Save Preset",
            juce::File{},
            "*.vstpreset;*.preset"
        );

        chooser->launchAsync(juce::FileBrowserComponent::saveMode |
                           juce::FileBrowserComponent::canSelectFiles,
            [this, processor](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file != juce::File{})
                {
                    // If no extension provided, default to .vstpreset
                    if (file.getFileExtension().isEmpty())
                        file = file.withFileExtension(".vstpreset");
                    
                    // Check directory exists and is writable
                    auto directory = file.getParentDirectory();
                    if (!directory.exists())
                    {
                        if (!directory.createDirectory())
                        {
                            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                "Save Error", "Cannot create directory: " + directory.getFullPathName());
                            return;
                        }
                    }
                    
                    // Check write permissions - for existing files check the file,
                    // for new files check the directory
                    bool hasWriteAccess = file.exists() ? 
                        file.hasWriteAccess() : 
                        directory.hasWriteAccess();
                        
                    if (!hasWriteAccess)
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                            "Save Error", 
                            "No write permission for file: " + file.getFullPathName());
                        return;
                    }
                    
                    // Fallback to generic preset saving
                    juce::MemoryBlock data;
                    processor->getStateInformation(data);
                    
                    // Write the preset data
                    if (!file.replaceWithData(data.getData(), data.getSize()))
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                            "Save Error", "Failed to write preset file: " + file.getFullPathName());
                    }
                }
            });
    }
}

juce::AudioProcessor* MainComponent::getActiveProcessor()
{
    // Find the plugin node in the graph
    for (auto* node : processorGraph.getNodes())
    {
        // Skip audio/MIDI IO nodes
        if (node->getProcessor()->getName() != "Audio Input" && 
            node->getProcessor()->getName() != "Audio Output" && 
            node->getProcessor()->getName() != "MIDI Input")
            return node->getProcessor();
    }
    return nullptr;
}