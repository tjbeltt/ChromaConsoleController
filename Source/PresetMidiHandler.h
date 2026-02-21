/*
  ==============================================================================

    PresetMidiHandler.h
    Created: 12 Feb 2026 11:40:11am
    Author:  tjbac

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"

/*
Midi Handler for triggering preset loading via MIDI notes
*/

class PresetMidiHandler
{
public:
    PresetMidiHandler(PresetManager& pm);
    ~PresetMidiHandler() = default;

    // Process incoming MIDI Messages
    // Call this from your AudioProcessor::processBlock()
    void processMidiMessages(juce::MidiBuffer& midiMessages, int numSamples);

    // Enable/Disable MIDI preset switching
    void setEnabled(bool shouldBeEnabled);
    bool isEnabled() const { return enabled.load(); }

    // Set MIDI channel to listen on 1-16 or 0 for all channels
    void setMidiChannel(int channel);
    int getMidiChannel() const { return midiChannel.load(); };

    // Gate velocity threshold for triggering (0-127)
    void setVelocityThreshold(int threshold);
    int getVelocityThreshold() const { return velocityThreshold.load(); };

    // Enable/Disable learning mode
    // When enabled, next MIDI note will be assigned to current preset
    void setLearningMode(bool shouldLearn);
    bool isLearningMode() const { return learningMode.load(); };

private:
    void handleNoteOn(int noteNumber, int velocity, int channel);

    PresetManager& presetManager;

    std::atomic<bool> enabled{ true };
    std::atomic<int> midiChannel{ 0 }; // 0 = all channels
    std::atomic<int> velocityThreshold{ 1 };
    std::atomic<bool> learningMode{ false };

    // Thread-safe set for tracking currently pressed notes
    juce::CriticalSection notesLock;
    std::set<int> activeNotes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMidiHandler)
};