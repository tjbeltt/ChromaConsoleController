/*
  ==============================================================================

    PresetBrowserComponent.h
    Created: 12 Feb 2026 1:08:11pm
    Author:  tjbac

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"

/*
UI Component for browsing and managing presets
Provides user-friendly interface to the PresetManager
*/

class PresetBrowserComponent : public juce::Component,
    public PresetManager::Listener,
    private juce::Timer
{
public:
    PresetBrowserComponent(PresetManager& pm);
    ~PresetBrowserComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void presetLoaded(const PresetManager::Preset& preset) override;
    void presetSaved(const PresetManager::Preset& preset) override;
    void presetListChanged() override;
    void currentPresetChanged() override;
private:
    void timerCallback() override;
    void updatePresetList();
    void showSavePresetDialog();
    void showDeletePresetDialog();
    void showMidiMappingDialog();

    PresetManager& presetManager;

    // UI Components
    juce::ComboBox categorySelector;
    juce::ListBox presetListBox;
    juce::TextButton previousButton;
    juce::TextButton nextButton;
    juce::TextButton saveButton;
    juce::TextButton deleteButton;
    juce::TextButton midiMapButton;
    juce::Label currentPresetLabel;

    // Preset List Model
    class PresetListBoxModel;
    std::unique_ptr<PresetListBoxModel> listBoxModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserComponent)
};

// Listbox Model for displaying presets

class PresetBrowserComponent::PresetListBoxModel : public juce::ListBoxModel
{
public:
    PresetListBoxModel(PresetManager& pm) : presetManager(pm) {}

    int getNumRows() override
    {
        return filteredPresets.size();
    }

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber >= filteredPresets.size()) { return; }

        auto& preset = filteredPresets[rowNumber];
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue);
        else if (rowNumber % 2 == 0)
            g.fillAll(juce::Colour(0xff2a2a2a));
        else
            g.fillAll(juce::Colour(0xff242424));

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);

        // Draw preset name
        g.drawText(preset.name, 10, 0, width - 60, height, juce::Justification::centredLeft);

        // Draw Midi indicator if mapped
        if (preset.midiNote >= 0)
        {
            g.setColour(juce::Colours::yellow);
            g.drawText("M" + juce::String(preset.midiNote), width - 50, 0, 45, height, juce::Justification::centredRight);
        }
    }

    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (row >= 0 && row < filteredPresets.size())
        {
            presetManager.loadPreset(filteredPresets[row].file);
        }
    }

    void setFilteredPresets(const juce::Array<PresetManager::Preset>& presets)
    {
        filteredPresets = presets;
    }

    const juce::Array<PresetManager::Preset>& getFilteredPresets() const
    {
        return filteredPresets;
    }

private:
    PresetManager& presetManager;
    juce::Array<PresetManager::Preset> filteredPresets;
};