/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ChildSlider.h"

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

    void runColumnOVC(int column, bool first, bool second, bool third, bool fourth);
    void setColumnProperties(int column, int value, bool first, bool second, bool third, bool fourth);
    void setColumnEnabled(int column, bool enabled, bool first, bool second, bool third, bool fourth);
    void setColumnColour(int colourID, juce::Colour colour, int column, bool first, bool second, bool third, bool fourth);
    void setColumnColour(std::vector<int> colourIds, juce::Colour colour, int column, bool first, bool second, bool third, bool fourth);

private:
    ChromaConsoleControllerAudioProcessor& audioProcessor;

    // UI Components
    juce::ComboBox channelSelector;
    juce::TextButton updateButton;
    std::vector<std::unique_ptr<juce::Slider>> ccSliders;
    std::vector<std::unique_ptr<juce::Label>> ccLabels;
    std::vector<int> sliderColourIds = { juce::Slider::thumbColourId, juce::Slider::trackColourId , juce::Slider::backgroundColourId };


    // Attachments
    juce::AudioProcessorValueTreeState::ComboBoxAttachment channelAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment updateAttachment;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> ccAttachments;

    void createCCControl(const CCControllerConfig& config);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChromaConsoleControllerAudioProcessorEditor)
};
