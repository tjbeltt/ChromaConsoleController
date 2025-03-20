/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const std::vector<CCControllerConfig> ChromaConsoleControllerAudioProcessor::ccConfigurations = {
     // Modules
    { 16, "cModule", "Character Module", 0}, // Character Module
    { 17, "mModule", "Movement Module", 22}, // Movement Module
    { 18, "dModule", "Diffusion Module", 44}, // Diffusion Module
    { 19, "tModule", "Texture Module", 0}, // Texture Module
    
    // Primary Controls
    { 64,  "tilt",  "Tilt", 63},  // Tilt
    { 66, "rate", "Rate", 127}, // Rate
    { 68, "time", "Time", 127}, // Time

    { 71, "tAmount", "Amount (Texture)", 63}, // Texture Amount
    

    { 65, "cAmount", "Amount (Character)", 63}, // Character Amount
    { 67, "mAmount", "Amount (Movement)", 63}, // Movement Amount
    { 69, "dAmount", "Amount (Diffusion)", 63}, // Diffusion Amount
    
    { 79, "tVol", "Effect Volume (Texture)", 63}, // Texture Volume
    

    // Secondary Controls
    { 72, "sensitivity", "Sensitivity", 63}, // Sensitivity
    { 74, "mDrift", "Drift (Movement)", 63}, // Movement Drift
    { 76, "dDrift", "Drift (Diffusion)", 63}, // Diffusion Drift
    { 70, "mix", "Mix", 127}, // Mix
    

    { 73, "cVol", "Effect Volume (Character)", 63}, // Character Volume
    { 75, "mVol", "Effect Volume (Movement)", 63}, // Movement Volume
    { 77, "dVol", "Effect Volume (Diffusion)", 63}, // Diffusion Volume
    { 78, "level", "Output Level", 127}, // Output Level
    

    // Bypass Controls
    { 91, "bypass1", "Standard Bypass", 127}, // Standard Bypass
    { 92, "bypass2", "Dual Bypass", 127}, // Dual Bypass

    // Other Functions
    { 82, "capture", "Capture", 0}, // Capture
    { 84, "filterMode", "Filter Mode", 63}, // Filter Mode
    { 80, "gesturePlayRec", "Gesture Play/Record", 0}, // Gesture Play/Record
    { 81, "gestureStopErase", "Gesture Stop/Erase", 127}, // Gesture Stop/Erase
    { 83, "captureRouting", "Capture Routing", 0}, // Capture Routing
    //{ 93, "taptempo", "Tap Tempo", 0}, // Tap Tempo (This tends to interrupt the midi clock being sent from my daw. I've disabled this for now)
    { 94, "calibrationLevel", "Calibration Level", 63}, // Calibration Level
    //{ 95, "calibrationMenu", "Calibration Menu (Enter)", 0} // Filter Mode // I also disabled this for UI cleanup's sake. 

};

//==============================================================================
ChromaConsoleControllerAudioProcessor::ChromaConsoleControllerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // Initialize previous values (?)
    for (const auto& config : ccConfigurations) {
        previousCCValues[config.ccNumber] = -1;
    }
}

ChromaConsoleControllerAudioProcessor::~ChromaConsoleControllerAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ChromaConsoleControllerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add MIDI channel parameter
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "midiChannel", "MIDI Channel",
        1, 16, 1));


    auto updateAttribute = juce::AudioParameterBoolAttributes().withStringFromValueFunction([](auto x, auto) { return x ? "Update" : "Off"; })
        .withLabel("enabled");
    layout.add(std::make_unique < juce::AudioParameterBool>(
        "updateValues", "Update Values", false, updateAttribute));

    auto characterModuleAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 21) { return juce::String("Drive"); }
        else if (x >= 22 && x <= 43) { return juce::String("Sweeten"); }
        else if (x >= 44 && x <= 65) { return juce::String("Fuzz"); }
        else if (x >= 66 && x <= 87) { return juce::String("Howl"); }
        else if (x >= 88 && x <= 109) { return juce::String("Swell"); }
        else return juce::String("Off");
        });

    auto movementModuleAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 21) { return juce::String("Doubler"); }
        else if (x >= 22 && x <= 43) { return juce::String("Vibrato"); }
        else if (x >= 44 && x <= 65) { return juce::String("Phaser"); }
        else if (x >= 66 && x <= 87) { return juce::String("Tremolo"); }
        else if (x >= 88 && x <= 109) { return juce::String("Pitch"); }
        else return juce::String("Off");
        });

    auto diffusionModuleAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 21) { return juce::String("Cascade"); }
        else if (x >= 22 && x <= 43) { return juce::String("Reels"); }
        else if (x >= 44 && x <= 65) { return juce::String("Space"); }
        else if (x >= 66 && x <= 87) { return juce::String("Collage"); }
        else if (x >= 88 && x <= 109) { return juce::String("Reverse"); }
        else return juce::String("Off");
        });

    auto textureModuleAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 21) { return juce::String("Filter"); }
        else if (x >= 22 && x <= 43) { return juce::String("Squash"); }
        else if (x >= 44 && x <= 65) { return juce::String("Cassette"); }
        else if (x >= 66 && x <= 87) { return juce::String("Broken"); }
        else if (x >= 88 && x <= 109) { return juce::String("Interference"); }
        else return juce::String("Off");
        });

    auto standardBypassAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 63) { return juce::String("Bypass"); }
        else return juce::String("Engage");
        });

    auto dualBypassAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 31) { return juce::String("Total Bypass"); }
        else if (x >= 32 && x <= 64) { return juce::String("Dual Bypass"); }
        else return juce::String("Total Engage");
        });

    auto gesturePlayRecAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 63) { return juce::String("Play"); }
        else return juce::String("Record");
        });

    auto gestureStopEraseAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 63) { return juce::String("Stop"); }
        else return juce::String("Erase");
        });

    auto captureAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 43) { return juce::String("Stop/Clear"); }
        else if (x >= 44 && x <= 87) { return juce::String("Play"); }
        else return juce::String("Record");
        });

    auto captureRoutingAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 63) { return juce::String("Post-FX"); }
        else return juce::String("Pre-FX");
        });

    auto filterAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 43) { return juce::String("LPF"); }
        else if (x >= 44 && x <= 87) { return juce::String("Tilt"); }
        else return juce::String("HPF");
        });

    auto calibrationLevelAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 31) { return juce::String("Low"); }
        else if (x >= 32 && x <= 63) { return juce::String("Medium"); }
        else if (x >= 64 && x <= 95) { return juce::String("High"); }
        else return juce::String("Very High");
        });

    auto calibrationMenuAttribute = juce::AudioParameterIntAttributes().withStringFromValueFunction([](auto x, auto) {
        if (x <= 63) { return juce::String("Exit"); }
        else return juce::String("Enter");
        });

    // Create parameters for each CC
    for (const auto& config : ccConfigurations) {
        // Character Module
        if (config.parameterID == "cModule") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), characterModuleAttribute));
        } 
        else if (config.parameterID == "mModule") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), movementModuleAttribute));
        }
        else if (config.parameterID == "dModule") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), diffusionModuleAttribute));
        }
        else if (config.parameterID == "tModule") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), textureModuleAttribute));
        }
        else if (config.parameterID == "bypass1") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), standardBypassAttribute));
        }
        else if (config.parameterID == "bypass2") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), dualBypassAttribute));
        }
        else if (config.parameterID == "gesturePlayRec") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), gesturePlayRecAttribute));
        }
        else if (config.parameterID == "gestureStopErase") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), gestureStopEraseAttribute));
        }
        else if (config.parameterID == "capture") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), captureAttribute));
        }
        else if (config.parameterID == "captureRouting") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), captureRoutingAttribute));
        }
        else if (config.parameterID == "filterMode") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), filterAttribute));
        }
        else if (config.parameterID == "calibrationLevel") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), calibrationLevelAttribute));
        }
        else if (config.parameterID == "calibrationMenu") {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue), calibrationMenuAttribute));
        }
        else {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                config.parameterID, config.name,
                0, 127, static_cast<int>(config.defaultValue)));
        }
    }


    return layout;
}

//==============================================================================
const juce::String ChromaConsoleControllerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChromaConsoleControllerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChromaConsoleControllerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChromaConsoleControllerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChromaConsoleControllerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChromaConsoleControllerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChromaConsoleControllerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChromaConsoleControllerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChromaConsoleControllerAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChromaConsoleControllerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChromaConsoleControllerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sendCurrentSliderValues();
}

void ChromaConsoleControllerAudioProcessor::sendCurrentSliderValues()
{
    for (const auto& config : ccConfigurations) {
        {
            // Get the current value of the slider
            const int currentValue = *parameters.getRawParameterValue(config.parameterID);
            // Create a MIDI CC message
            juce::MidiMessage midiMessage = juce::MidiMessage::controllerEvent(getMidiChannel(), config.ccNumber, currentValue);
            sendMidiMessage(midiMessage); // Add the message to the queue
        }
    }
}

void ChromaConsoleControllerAudioProcessor::sendMidiMessage(const juce::MidiMessage& message)
{
    pendingMidiMessages.addEvent(message, 0); // Add the message to the buffer
}

void ChromaConsoleControllerAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChromaConsoleControllerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ChromaConsoleControllerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    midiMessages.clear();
    const int midiChannel = getMidiChannel();

    // Check for pending MIDI messages
    {
        if (!pendingMidiMessages.isEmpty())
        {
            midiMessages.addEvents(pendingMidiMessages, 0, buffer.getNumSamples(), 0); // Add pending messages
            pendingMidiMessages.clear(); // Clear the pending buffer
        }

    }

    for (const auto& config : ccConfigurations) {
        const int currentValue = *parameters.getRawParameterValue(config.parameterID);
        int& prevValue = previousCCValues[config.ccNumber];

        if (currentValue != prevValue) {
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(
                midiChannel, config.ccNumber, currentValue), 0);
            prevValue = currentValue;
        }
    }
}

//==============================================================================
bool ChromaConsoleControllerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChromaConsoleControllerAudioProcessor::createEditor()
{
    return new ChromaConsoleControllerAudioProcessorEditor (*this);
}

//==============================================================================
void ChromaConsoleControllerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ChromaConsoleControllerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChromaConsoleControllerAudioProcessor();
}
