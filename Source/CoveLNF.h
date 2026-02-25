/*
  ==============================================================================

    CoveLNF.h
    Created: 5 Dec 2025 3:06:11pm
    Author:  tjbac

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CoveLNF : public juce::LookAndFeel_V4
{

public:
    enum CoveRotarySlider
    {
        backgroundArcID = 0x1000000,
        thumbEnabledID = 0x1000001,
        thumbDisabledID = 0x1000002

    };

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider&) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;
};