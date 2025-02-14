/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ChromaConsoleControllerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ChromaConsoleControllerAudioProcessorEditor (ChromaConsoleControllerAudioProcessor&);
    ~ChromaConsoleControllerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ChromaConsoleControllerAudioProcessor& audioProcessor;

    // UI Components
    juce::ComboBox channelSelector;
    juce::TextButton updateButton;
    std::vector<std::unique_ptr<juce::Slider>> ccSliders;
    std::vector<std::unique_ptr<juce::Label>> ccLabels;

    // Attachments
    juce::AudioProcessorValueTreeState::ComboBoxAttachment channelAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment updateAttachment;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> ccAttachments;

    void createCCControl(const CCControllerConfig& config);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChromaConsoleControllerAudioProcessorEditor)
};
