/*
  ==============================================================================

    MidiLearnDialog.h
    Created: 1 Mar 2026 7:52:11pm
    Author:  tjbac

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"

// Custom dialog for MIDI note learning
class MidiLearnDialog : public juce::Component,
    public juce::Timer
{
public:
    MidiLearnDialog(PresetManager& pm, const juce::File& presetFile, int currentNote)
        : presetManager(pm),
        presetFileToMap(presetFile),
        originalNote(currentNote)
    {
        // Instructions label
        instructionsLabel.setText("Enter MIDI note (0-127) or click Learn", juce::dontSendNotification);
        instructionsLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(instructionsLabel);

        // MIDI note text editor
        noteEditor.setText(currentNote >= 0 ? juce::String(currentNote) : "", false);
        noteEditor.setJustification(juce::Justification::centred);
        noteEditor.setInputRestrictions(3, "0123456789");
        addAndMakeVisible(noteEditor);

        // Learn Button
        learnButton.setButtonText("Learn");
        learnButton.onClick = [this]() { startLearning(); };
        addAndMakeVisible(learnButton);

        // OK Button
        okButton.setButtonText("OK");
        okButton.onClick = [this]() { applyAndClose(); };
        addAndMakeVisible(okButton);

        // Cancel Button
        cancelButton.setButtonText("Cancel");
        cancelButton.onClick = [this]() { cancelAndClose(); };
        addAndMakeVisible(cancelButton);

        // Status label (hidden on init)
        statusLabel.setText("", juce::dontSendNotification);
        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        addAndMakeVisible(statusLabel);

        setSize(400, 170);
    }

    ~MidiLearnDialog() override
    {
        stopLearning();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff2a2a2a));

        // Border
        g.setColour(juce::Colour(0xff404040));
        g.drawRect(getLocalBounds(), 2);

        if (isLearning)
        {
            float alpha = 0.3f + 0.2f * std::sin(pulsePhase);
            g.setColour(juce::Colours::yellow.withAlpha(alpha));
            g.fillRect(getLocalBounds().reduced(10));

            g.setColour(juce::Colours::yellow);
            g.drawRect(getLocalBounds().reduced(10), 3);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);

        // Labels

        instructionsLabel.setBounds(area.removeFromTop(25));
        area.removeFromTop(10);

        // Only visible when learning
        statusLabel.setBounds(area.removeFromTop(30)); 
        area.removeFromTop(10);

        noteEditor.setBounds(area.removeFromTop(30).reduced(80, 0));
        area.removeFromTop(20);

        // Buttons

        auto buttonArea = area.removeFromTop(35);
        int buttonWidth = (buttonArea.getWidth() - 20) / 3;

        learnButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
        buttonArea.removeFromLeft(10);
        
        okButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
        buttonArea.removeFromLeft(10);

        cancelButton.setBounds(buttonArea);
    }

    void startLearning()
    {
        if (isLearning)
        {
            stopLearning();
            return;
        }

        isLearning = true;
        pulsePhase = 0.0f;
        learnButton.setButtonText("Stop Learning");
        statusLabel.setText("Waiting for MIDI note ...", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);

        // Disable other controls
        noteEditor.setEnabled(false);
        okButton.setEnabled(false);


        startTimer(50);
        startListeningForMidi();
    }


    void stopLearning()
    {
        if (!isLearning)
            return;

        isLearning = false;
        learnButton.setButtonText("Learn");
        statusLabel.setText("", juce::dontSendNotification);

        // Re-Enable controls
        noteEditor.setEnabled(true);
        okButton.setEnabled(true);
        //cancelButton.setEnabled(true);

        stopTimer();
        stopListeningForMidi();
        repaint();
    }

    void midiNoteReceived(int noteNumber)
    {
        if (!isLearning)
            return;

        // Update text editor
        noteEditor.setText(juce::String(noteNumber), false);

        // Show Success
        statusLabel.setText("Learned Note: " + juce::String(noteNumber), juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::green);

        stopLearning();

        // Auto apply after delay
        juce::Timer::callAfterDelay(500, [safeThis = juce::Component::SafePointer<MidiLearnDialog>(this)]() {
            if (safeThis != nullptr && safeThis->isShowing())
                safeThis->applyAndClose();
            });
    }

    std::function<void()> onClose;

private:
    void timerCallback() override
    {
        pulsePhase += 0.15f;
        if (pulsePhase > juce::MathConstants<float>::twoPi)
            pulsePhase -= juce::MathConstants<float>::twoPi;

        repaint();
    }

    void startListeningForMidi()
    {
        // This will be called from the audio thread via a callback
        midiCallback = [this](int note) {
            juce::MessageManager::callAsync([this, note]() {
                if (isShowing())
                    midiNoteReceived(note);
                });
            };
    }

    void stopListeningForMidi()
    {
        midiCallback = nullptr;
    }

    void updateCancelButtonText()
    {
        if (originalNote >= 0)
            cancelButton.setButtonText("Clear Mapping");
        else
            cancelButton.setButtonText("Cancel");
    }

    void cancelAndClose()
    {
        if (originalNote >= 0)
        {
            presetManager.clearMidiMapping(originalNote);
        }

        closeDialog();
    }

    void applyAndClose()
    {
        auto noteText = noteEditor.getText();
        
        // Clear Mapping if empty
        if (noteText.isEmpty())
        {
            if (originalNote >= 0)
                presetManager.clearMidiMapping(originalNote);
            closeDialog();
            return;
        }

        int note = noteText.getIntValue();
        if (note >= 0 && note <= 127)
            presetManager.setMidiNoteForPreset(presetFileToMap, note);
     
        closeDialog();
    
    }

    void closeDialog()
    {
        stopLearning();
        if (onClose) onClose();

        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    }

    PresetManager& presetManager;
    juce::File presetFileToMap;
    int originalNote;

    juce::Label instructionsLabel;
    juce::Label statusLabel;
    juce::TextEditor noteEditor;
    juce::TextButton learnButton;
    juce::TextButton okButton;
    juce::TextButton cancelButton;

    bool isLearning = false;
    float pulsePhase = 0.0f;

    std::function<void(int)> midiCallback;
};
