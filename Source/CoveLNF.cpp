/*
  ==============================================================================

    CoveLNF.cpp
    Created: 5 Dec 2025 3:06:11pm
    Author:  tjbac

  ==============================================================================
*/

#include "CoveLNF.h"

struct DebugLabel : public juce::Label
{
    void colourChanged() override
    {
        DBG("DebugLabel::colourChanged called");
        juce::Logger::writeToLog(juce::SystemStats::getStackBacktrace());
        juce::Label::colourChanged();
    }
};

void CoveLNF::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto fill    = slider.findColour(juce::Slider::rotarySliderFillColourId);
    auto bgArc   = slider.findColour(CoveLNF::CoveRotarySlider::backgroundArcID);
    auto bodyCol = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    auto thumbOn = slider.findColour(CoveLNF::CoveRotarySlider::thumbEnabledID);
    auto thumbOff= slider.findColour(CoveLNF::CoveRotarySlider::thumbDisabledID);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4);
    auto cx = bounds.getCentreX();
    auto cy = bounds.getCentreY();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float halfPi = juce::MathConstants<float>::halfPi;

    // Tick marks (panel-style markings around the knob)
    const int numTicks = 21;
    const float tickOuter = radius;
    const float tickLen   = radius * 0.13f;
    const float tickW     = juce::jmax(1.0f, radius * 0.045f);

    for (int i = 0; i < numTicks; ++i)
    {
        float t = (float)i / (float)(numTicks - 1);
        float angle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        bool active = slider.isEnabled() && (angle <= toAngle + 0.001f);
        g.setColour(active ? fill : bgArc);

        float cosA = std::cos(angle - halfPi);
        float sinA = std::sin(angle - halfPi);
        g.drawLine(cx + (tickOuter - tickLen) * cosA, cy + (tickOuter - tickLen) * sinA,
                   cx + tickOuter * cosA,              cy + tickOuter * sinA,
                   tickW);
    }

    // Knob body (dark circle representing the physical knob)
    float bodyR = radius * 0.62f;
    g.setColour(bodyCol);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // Colored cap (flat colored top, matches hardware column color)
    float capR = radius * 0.44f;
    g.setColour(slider.isEnabled() ? fill : thumbOff);
    g.fillEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

    // Position indicator line on the cap
    if (slider.isEnabled())
    {
        float cosA = std::cos(toAngle - halfPi);
        float sinA = std::sin(toAngle - halfPi);
        g.setColour(thumbOn);
        g.drawLine(cx + capR * 0.15f * cosA, cy + capR * 0.15f * sinA,
                   cx + capR * 0.82f * cosA, cy + capR * 0.82f * sinA,
                   juce::jmax(1.5f, radius * 0.05f));
    }
}

juce::Label* CoveLNF::createSliderTextBox(juce::Slider& slider)
{
    auto* l = LookAndFeel_V4::createSliderTextBox(slider);
    //auto* l = new DebugLabel();

    l->setColour(juce::Label::textColourId,
        slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::Label::backgroundColourId,
        slider.findColour(juce::Slider::textBoxBackgroundColourId));
    l->setColour(juce::Label::outlineColourId,
        slider.findColour(juce::Slider::textBoxOutlineColourId));

    return l;
}