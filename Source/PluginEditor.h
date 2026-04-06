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
#include "CoveLNF.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "PresetBrowserComponent.h"
#include "PresetMidiHandler.h"
#include "UpdateChecker.h"

class ChromaConsoleControllerAudioProcessorEditor : public juce::AudioProcessorEditor,
	public UpdateChecker::Listener
{
public:
    ChromaConsoleControllerAudioProcessorEditor(ChromaConsoleControllerAudioProcessor&);
    ~ChromaConsoleControllerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ChromaConsoleControllerAudioProcessor& audioProcessor;
    CoveLNF lnf;

    // Updates
	UpdateChecker updateChecker;
    void updateAvailable(const juce::String& newVersion,
        const juce::String& downloadUrl,
        const juce::String& changelog) override;

    // UI Components
    juce::ComboBox channelSelector;
    juce::TextButton updateButton;
    juce::TextButton advancedButton;
    juce::TextButton autoUpdateButton;
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
        static const juce::Colour bg_dark;
        static const juce::Colour bg;
        static const juce::Colour bg_light;
        static const juce::Colour text;
        static const juce::Colour text_muted;
        static const juce::Colour highlight;
        static const juce::Colour border;
        static const juce::Colour border_muted;
        static const juce::Colour primary;
        static const juce::Colour secondary;
    };

    std::vector<int> sliderColourIds = {
        juce::Slider::rotarySliderFillColourId,
        //CoveLNF::CoveRotarySlider::thumbEnabledID,
        //CoveLNF::CoveRotarySlider::backgroundArcID,
        juce::Slider::textBoxTextColourId,
        juce::Slider::textBoxBackgroundColourId,
        juce::Slider::textBoxOutlineColourId
    };

    void setLAF(); // Function to set Look And Feel colors

    // Attachments
    juce::AudioProcessorValueTreeState::ComboBoxAttachment channelAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment updateAttachment;

    // Module management
    void createCCModules(); // Create sliders programatically from the amount of parameters defined in the audio processor
    void setupColumnInteractions(); // Setup value change callbacks for the first 4 modules (column headers)
    void toggleAdvancedSettings(); // Toggle visibility of the advanced settings

    // Column control methods
    void setColumnProperties(int column, int value, bool first, bool second, bool third, bool fourth); // Setter for managing enabled state and color of columns
    void setColumnEnabled(int column, bool enabled, bool first, bool second, bool third, bool fourth); // Setter for toggling enabled state of a column with individual slider control aswell.
    void setColumnColour(int column, juce::Colour colour, bool first, bool second, bool third, bool fourth); // Setter for the colour of the sliders in the columns
    void setColumnTextBoxColour(int column, juce::Colour textColour, juce::Colour bgColour, juce::Colour outlineColour,
        bool first, bool second, bool third, bool fourth);

    // Helper methods
    int calculateNumRows() const;
    int calculateVisibleRows() const;
    int calculateIdealHeight() const;
    juce::Colour getColourForValue(int value) const;
    bool getShowAdvancedSettings() const;
    void setShowAdvancedSettings(bool show);
    bool getAutoCheckForUpdates() const;
    void setAutoCheckForUpdates(bool enabled);
    bool getPresetBrowserVisible() const;
	void setPresetBrowserVisible(bool visible);
    

    // Preset Browser
    juce::TextButton presetButton;          // Button to show/hide preset browser
    PresetBrowserComponent presetBrowser;   
    bool showPresetBrowser = false;         // Visibility state
    static constexpr int presetBrowserWidth = 300;

    void togglePresetBrowser();

    //melatonin::Inspector inspector{ *this, false };
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChromaConsoleControllerAudioProcessorEditor)
};

/*
--bg-dark: hsl(39 24% 1%);          hsl(40, 10, 6)
--bg: hsl(41 15% 4%);               hsl(48, 6, 15)
--bg-light: hsl(41 8% 8%);          hsl(48, 6, 17)
--text: hsl(41 33% 94%);            hsl(26, 8, 82)
--text-muted: hsl(41 6% 69%);       hsl(34, 3, 42)
--highlight: hsl(41 5% 38%);        hsl(38, 3, 47)
--border: hsl(41 6% 27%);           hsl(26, 4, 63)
--border-muted: hsl(41 9% 17%);     hsl(60, 2, 34)
--primary: hsl(42 52% 60%);         hsl(3, 46, 50)
--secondary: hsl(222 77% 76%);      hsl(192, 99, 58)
--danger: hsl(9 26% 64%);
--warning: hsl(52 19% 57%);
--success: hsl(146 17% 59%);
--info: hsl(217 28% 65%);
*/