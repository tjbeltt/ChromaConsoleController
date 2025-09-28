/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    Cleaner modular approach with CCSliderModule components

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CCSliderModule.h"

class ChromaConsoleControllerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ChromaConsoleControllerAudioProcessorEditor(ChromaConsoleControllerAudioProcessor&);
    ~ChromaConsoleControllerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ChromaConsoleControllerAudioProcessor& audioProcessor;

    // UI Components
    juce::ComboBox channelSelector;
    juce::TextButton updateButton;
    juce::TextButton advancedButton;
    juce::Label versionNumber;
    juce::ComponentBoundsConstrainer constrainer;
    

    // Modular slider components
    std::vector<std::unique_ptr<CCSliderModule>> ccModules;
    

    // Layout properties
    static constexpr int numColumns = 4;
    static constexpr int headerHeight = 25;
    static constexpr int footerHeight = 25;
    static constexpr int padding = 10;
    static constexpr int basicRows = 5;

    // Color definitions
    struct Colors {
        static const juce::Colour red;
        static const juce::Colour yellow;
        static const juce::Colour green;
        static const juce::Colour blue;
        static const juce::Colour purple;
    };

    std::vector<int> sliderColourIds = {
        juce::Slider::thumbColourId,
        juce::Slider::trackColourId,
        juce::Slider::backgroundColourId,
        juce::Slider::rotarySliderFillColourId
    };

    // Attachments
    juce::AudioProcessorValueTreeState::ComboBoxAttachment channelAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment updateAttachment;

    // Module management
    void createCCModules();
    void setupColumnInteractions();
    void toggleAdvancedSettings();

    // Column control methods (refactored for modules)
    void setColumnProperties(int column, int value, bool first, bool second, bool third, bool fourth);
    void setColumnEnabled(int column, bool enabled, bool first, bool second, bool third, bool fourth);
    void setColumnColour(int column, juce::Colour colour, bool first, bool second, bool third, bool fourth);

    // Helper methods
    int calculateNumRows() const;
    int calculateVisibleRows() const;
    int calculateIdealHeight() const;
    juce::Rectangle<int> getGridArea() const;
    juce::Colour getColourForValue(int value) const;
    bool getShowAdvancedSettings() const;
    void setShowAdvancedSettings(bool show);
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChromaConsoleControllerAudioProcessorEditor)
};