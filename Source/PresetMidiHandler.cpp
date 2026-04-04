/*
  ==============================================================================

    PresetMidiHandler.cpp
    Created: 12 Feb 2026 11:40:11am
    Author:  tjbac

  ==============================================================================
*/

#include "PresetMidiHandler.h"

PresetMidiHandler::PresetMidiHandler(PresetManager& pm) : presetManager(pm)
{
}

void PresetMidiHandler::processMidiMessages(juce::MidiBuffer& midiMessages, int numSamples)
{
    if (!enabled.load())
        return;

    juce::MidiBuffer::Iterator i(midiMessages);
    juce::MidiMessage msg;
    int samplePosition;

    while (i.getNextEvent(msg, samplePosition))
    {
        if (msg.isNoteOn())
        {
            int channel = msg.getChannel();
            int targetChannel = midiChannel.load();

            // Check if we should process this channel
            if (targetChannel == 0 || channel == targetChannel)
                handleNoteOn(msg.getNoteNumber(), msg.getVelocity(), channel);
        }
        else if (msg.isNoteOff())
        {
            // Remove from active notes
            const juce::ScopedLock sl(notesLock);
            activeNotes.erase(msg.getNoteNumber());
        }
    }
}

void PresetMidiHandler::setEnabled(bool shouldBeEnabled)
{
    enabled.store(shouldBeEnabled);

    if (!shouldBeEnabled)
    {
        // Clear active notes when disabled
        const juce::ScopedLock sl(notesLock);
        activeNotes.clear();
    }
}

void PresetMidiHandler::setMidiChannel(int channel)
{
    // Clamp to valid range (0-16 where 0 means all channels)
    midiChannel.store(juce::jlimit(0, 16, channel));
}

void PresetMidiHandler::setVelocityThreshold(int threshold)
{
    velocityThreshold.store(juce::jlimit(0, 127, threshold));
}

void PresetMidiHandler::setLearningMode(bool shouldLearn)
{
    learningMode.store(shouldLearn);
}

void PresetMidiHandler::setMidiLearnCallback(std::function <void(int)> callback)
{
    const juce::ScopedLock sl(callBackLock);
    midiLearnCallback = callback;
}

void PresetMidiHandler::clearMidiLearnCallback()
{
    const juce::ScopedLock sl(callBackLock);
    midiLearnCallback = nullptr;
}

void PresetMidiHandler::handleNoteOn(int noteNumber, int velocity, int channel)
{
    // Check velocity threshold
    if (velocity < velocityThreshold.load())
        return;

    // Prevent retriggering if note is already active
    {
        const juce::ScopedLock sl(notesLock);
        if (activeNotes.count(noteNumber) > 0)
            return;
        activeNotes.insert(noteNumber);
    }
    
    // Check if there's a MIDI learn callback active
    {
        const juce::ScopedLock sl(callBackLock);
        if (midiLearnCallback)
        {
            midiLearnCallback(noteNumber);
            return; // Don't process any further when learning for UI
        }
    }

    if (learningMode.load())
    {
        auto* currentPreset = presetManager.getCurrentPreset();
        if (currentPreset)
        {
            presetManager.setMidiNoteForPreset(currentPreset->file, noteNumber);

            // Auto disable learning mode after assignment
            learningMode.store(false);

            DBG("MIDI note " << noteNumber << "assigned to preset: " << currentPreset->name);
        }
        return;
    }

    // Normal mode: Load preset mapped to this note
    bool loaded = presetManager.loadPresetFromMidiNote(noteNumber);

    if (loaded)
    {
        DBG("Loaded preset from MIDI note: " << noteNumber);
    }
}