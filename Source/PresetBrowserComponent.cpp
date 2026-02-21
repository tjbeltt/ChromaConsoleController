/*
  ==============================================================================

    PresetBrowserComponent.cpp
    Created: 12 Feb 2026 1:08:11pm
    Author:  tjbac

  ==============================================================================
*/

#include "PresetBrowserComponent.h"

PresetBrowserComponent::PresetBrowserComponent(PresetManager& pm)
    : presetManager(pm)
{
    // Create list box model
    listBoxModel = std::make_unique<PresetListBoxModel>(presetManager);

    // Setup category selector
    categorySelector.addItem("All Presets", 1);
    auto categories = presetManager.getCategories();
    for (int i = 0; i < categories.size(); i++)
        categorySelector.addItem(categories[i], i + 2);

    categorySelector.setSelectedId(1);
    categorySelector.onChange = [this]
        {
            updatePresetList();
        };
    addAndMakeVisible(categorySelector);

    // Set up preset list
    presetListBox.setModel(listBoxModel.get());
    presetListBox.setRowHeight(30);
    presetListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1a1a1a));
    addAndMakeVisible(presetListBox);

    // Set up navigation buttons
    previousButton.setButtonText("<");
    previousButton.onClick = [this] { presetManager.loadPreviousPreset(); };
    addAndMakeVisible(previousButton);

    nextButton.setButtonText(">");
    nextButton.onClick = [this] { presetManager.loadNextPreset(); };
    addAndMakeVisible(nextButton);

    // Set up action buttons
    saveButton.setButtonText("Save");
    saveButton.onClick = [this] { showSavePresetDialog(); };
    addAndMakeVisible(saveButton);

    deleteButton.setButtonText("Delete");
    deleteButton.onClick = [this] { showDeletePresetDialog(); };
    addAndMakeVisible(deleteButton);
    
    midiMapButton.setButtonText("MIDI Map");
    midiMapButton.onClick = [this] { showMidiMappingDialog(); };
    addAndMakeVisible(midiMapButton);

    // Setup current preset label
    currentPresetLabel.setText("No Preset Loaded", juce::dontSendNotification);
    currentPresetLabel.setJustificationType(juce::Justification::centred);
    currentPresetLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(currentPresetLabel);

    // Register as listener
    presetManager.addListener(this);

    // Initial update
    updatePresetList();

    // Start timer for UI updates
    startTimer(100);
}

PresetBrowserComponent::~PresetBrowserComponent()
{
    presetManager.removeListener(this);
    presetListBox.setModel(nullptr);
}

void PresetBrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff202020));

    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("Preset Browser", getLocalBounds().removeFromTop(40), juce::Justification::centred);
}

void PresetBrowserComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Title area
    bounds.removeFromTop(40);

    // CVurrent preset display
    currentPresetLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(5);

    // Navigation Buttons
    auto navArea = bounds.removeFromTop(30);
    previousButton.setBounds(navArea.removeFromLeft(40));
    navArea.removeFromLeft(5);
    nextButton.setBounds(navArea.removeFromLeft(40));
    bounds.removeFromTop(10);

    // Category Selector
    categorySelector.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(10);

    // Preset list (takes remaining space)
    auto listHeight = bounds.getHeight() - 45; // Leave room for buttons
    presetListBox.setBounds(bounds.removeFromTop(listHeight));
    bounds.removeFromTop(10);

    // Action buttons
    auto buttonArea = bounds.removeFromTop(30);
    auto buttonWidth = buttonArea.getWidth() / 3 - 5;

    saveButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    deleteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    midiMapButton.setBounds(buttonArea);

}

void PresetBrowserComponent::presetLoaded(const PresetManager::Preset& preset)
{
    currentPresetLabel.setText(preset.name + " (" + preset.category + ")", juce::dontSendNotification);
    presetListBox.repaint();
}

void PresetBrowserComponent::presetSaved(const PresetManager::Preset& preset)
{
    updatePresetList();
}

void PresetBrowserComponent::presetListChanged()
{
    // Update category list
    categorySelector.clear();
    categorySelector.addItem("All Presets", 1);
    auto categories = presetManager.getCategories();
    for (int i = 0; i < categories.size(); i++)
        categorySelector.addItem(categories[i], i + 2);

    updatePresetList();
}

void PresetBrowserComponent::currentPresetChanged()
{
    auto* currentPreset = presetManager.getCurrentPreset();
    if (currentPreset)
    {
        currentPresetLabel.setText(currentPreset->name + " (" + currentPreset->category + ")", juce::dontSendNotification);
    }

    // Update selection in list
    int currentIndex = presetManager.getCurrentPresetIndex();
    auto& filtered = listBoxModel->getFilteredPresets();

    for (int i = 0; i < filtered.size(); i++)
    {
        if (filtered[i].file == currentPreset->file)
        {
            presetListBox.selectRow(i);
            break;
        }
    }
}

void PresetBrowserComponent::timerCallback()
{
    // Enable/disable based on state
    auto* currentPreset = presetManager.getCurrentPreset();
    deleteButton.setEnabled(currentPreset != nullptr);
    midiMapButton.setEnabled(currentPreset != nullptr);
}

void PresetBrowserComponent::updatePresetList()
{
    juce::Array<PresetManager::Preset> presetsToShow;

    auto selectedCategory = categorySelector.getText();
    if (selectedCategory == "All Presets")
    {
        presetsToShow = presetManager.getAllPresets();
    }
    else
    {
        presetsToShow = presetManager.getPresetsByCategory(selectedCategory);
    }

    listBoxModel->setFilteredPresets(presetsToShow);
    presetListBox.updateContent();
    presetListBox.repaint();
}

void PresetBrowserComponent::showSavePresetDialog()
{
    // Create alert window for preset name input
    auto* window = new juce::AlertWindow("Save Preset", "Enter Preset Name:", juce::AlertWindow::NoIcon);

    window->addTextEditor("presetName", "", "Preset Name:");
    window->addComboBox("category", presetManager.getCategories(), "Category:");
    window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            if (result == 1)
            {
                auto presetName = window->getTextEditorContents("presetName");
                auto categoryBox = window->getComboBoxComponent("category");
                auto category = categoryBox ? categoryBox->getText() : "User";

                if (presetName.isNotEmpty())
                {
                    presetManager.savePreset(presetName, category);
                }
            }
        }), true);
}

void PresetBrowserComponent::showDeletePresetDialog()
{
    auto* currentPreset = presetManager.getCurrentPreset();
    if (!currentPreset)
        return;

    auto result = juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Delete Preset",
        "Are you sure you want to delete '" + currentPreset->name + "'?",
        "Delete",
        "Cancel",
        nullptr,
        nullptr
    );

    if (result)
    {
        presetManager.deletePreset(currentPreset->file);
    }
}

void PresetBrowserComponent::showMidiMappingDialog()
{
    auto* currentPreset = presetManager.getCurrentPreset();
    if (!currentPreset)
        return;

    auto* window = new juce::AlertWindow("MIDI Mapping",
        "Enter MIDI note (0-127) or -1 to clear:",
        juce::AlertWindow::NoIcon);

    auto currentNote = presetManager.getMidiNoteForPreset(currentPreset->file);
    window->addTextEditor("midiNote", juce::String(currentNote), "MIDI Note:");
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cance", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window, currentNote](int result)
    {
        if (result == 1)
        {
            auto noteText = window->getTextEditorContents("midiNote");
            int note = noteText.getIntValue();

            if (note >= -1 && note <= 127)
            {
                if (note == -1)
                    presetManager.clearMidiMapping(currentNote);
                else
                {
                    auto* preset = presetManager.getCurrentPreset();
                    if (preset)
                        presetManager.setMidiNoteForPreset(preset->file, note);
                }
                updatePresetList();
            }
        }

    }), true);
}