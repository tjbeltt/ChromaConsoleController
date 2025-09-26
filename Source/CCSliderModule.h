/*
  ==============================================================================

    CCSliderModule.h
    Created: 25 Sep 2025 5:03:08pm
    Author:  tjbac

    Created: Component for individual CC slider controls
    A self-contained module with slider, label, and attachment

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CCSliderModule : public juce::Component
{
public:
    CCSliderModule(ChromaConsoleControllerAudioProcessor& processor,
        const CCControllerConfig& config);
    ~CCSliderModule() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Slider access
    juce::Slider& getSlider() { return slider; }
    const juce::Slider& getSlider() const { return slider; }

    // Enable/disable the module
    void setEnabled(bool shouldBeEnabled);

    // Color management
    void setSliderColour(int colourId, juce::Colour colour);
    void setSliderColours(const std::vector<int>& colourIds, juce::Colour colour);

    // Get the parameter ID for this module
    const juce::String& getParameterID() const { return parameterID; }

private:
    ChromaConsoleControllerAudioProcessor& audioProcessor;

    // UI Components
    juce::Slider slider;
    juce::Label nameLabel;

    // Parameter info
    juce::String parameterID;

    // Attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCSliderModule)
};