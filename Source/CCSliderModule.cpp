/*
  ==============================================================================

    CCSliderModule.cpp
    Created: 25 Sep 2025 5:03:08pm
    Author:  tjbac

    Component for individual CC slider controls

  ==============================================================================
*/

#include "CCSliderModule.h"

CCSliderModule::CCSliderModule(ChromaConsoleControllerAudioProcessor& processor,
    const CCControllerConfig& config)
    : audioProcessor(processor), parameterID(config.parameterID)
{
    // Configure slider
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 15);
    slider.setRange(0, 127, 1);
    addAndMakeVisible(slider);

    // Configure label
    nameLabel.setText(config.name, juce::dontSendNotification);
    nameLabel.setFont(12);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(nameLabel);

    // Create attachment
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters,
        config.parameterID,
        slider
    );
}

CCSliderModule::~CCSliderModule()
{
}

void CCSliderModule::paint(juce::Graphics& g)
{
    // Optional: Add background or border for debugging
    //g.setColour(juce::Colours::darkgrey);
    //g.drawRect(getLocalBounds(), 1);
}

void CCSliderModule::resized()
{
    auto area = getLocalBounds();

    // Reserve space for the label at the top
    auto labelArea = area.removeFromBottom(proportionOfHeight(0.15f));
    nameLabel.setBounds(labelArea);

    // Use remaining space for slider
    slider.setBounds(area);
}

void CCSliderModule::setEnabled(bool shouldBeEnabled)
{
    Component::setEnabled(shouldBeEnabled);
    slider.setEnabled(shouldBeEnabled);
    nameLabel.setEnabled(shouldBeEnabled);
}

void CCSliderModule::setSliderColour(int colourId, juce::Colour colour)
{
    slider.setColour(colourId, colour);
}

void CCSliderModule::setSliderColours(const std::vector<int>& colourIds, juce::Colour colour)
{
    for (int colourId : colourIds)
    {
        slider.setColour(colourId, colour);
    }
}