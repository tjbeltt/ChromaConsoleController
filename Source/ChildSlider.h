/*
  ==============================================================================

    ChildSlider.h
    Created: 19 Mar 2025 4:26:41pm
    Author:  tjbac

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ChildSlider  : public juce::Slider, public juce::Slider::Listener
{
public:
    ChildSlider(juce::Slider* parentSlider) : parentSlider(parentSlider)
    {
        if (parentSlider == nullptr)
        {
            throw std::runtime_error("Parent slider pointer == NULL for Child Slider");
        }
        parentSlider->addListener(this);
        updateColour();

    }

    ~ChildSlider() override
    {
        parentSlider->removeListener(this);
    }

    void sliderValueChanged(juce::Slider* slider) override {} // Not needed for color changes

    void lookAndFeelChanged() override
    {
        // Update the child slider's color when the LookAndFeel changes
        updateColour();
    }

private:
    juce::Slider* parentSlider;

    void updateColour()
    {
        // Copy the parent slider's thumb, track, and background colors
        auto& lf = getLookAndFeel();
        setColour(juce::Slider::thumbColourId, parentSlider->findColour(juce::Slider::thumbColourId));
        setColour(juce::Slider::trackColourId, parentSlider->findColour(juce::Slider::trackColourId));
        setColour(juce::Slider::backgroundColourId, parentSlider->findColour(juce::Slider::backgroundColourId));
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildSlider)
};
