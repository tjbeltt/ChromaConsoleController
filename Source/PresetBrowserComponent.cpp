/*
  ==============================================================================

    PresetBrowserComponent.cpp
    Created: 12 Feb 2026 1:08:11pm
    Author:  tjbac

  ==============================================================================
*/

#include "PresetBrowserComponent.h"

PresetBrowserComponent::PresetBrowserComponent(PresetManager& pm, PresetMidiHandler& mh)
    : presetManager(pm), presetMidiHandler(mh)
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
    presetListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromHSL((0.0f), 0.00f, .04f, 1.0f));
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

    preserveMidiChannelButton.setButtonText("Safe");
    preserveMidiChannelButton.setClickingTogglesState(true);
    preserveMidiChannelButton.setToggleState(presetManager.getPreserveMidiChannel(), juce::dontSendNotification);
    preserveMidiChannelButton.onClick = [this]() {
        bool newState = preserveMidiChannelButton.getToggleState();
        presetManager.setPreserveMidiChannel(newState);
        updatePreserveMidiChannelButton();
        };
    addAndMakeVisible(preserveMidiChannelButton);

    // Setup current preset label
    currentPresetLabel.setText("No Preset Loaded", juce::dontSendNotification);
    currentPresetLabel.setJustificationType(juce::Justification::centred);
    currentPresetLabel.setFont(juce::Font(13.0f));
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
    g.fillAll(juce::Colour::fromHSL((0.0f), 0.00f, .15f, 1.0f));

    g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
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
    auto buttonWidth = (buttonArea.getWidth() -15) / 4;

    saveButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    deleteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    midiMapButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    preserveMidiChannelButton.setBounds(buttonArea);

}

void PresetBrowserComponent::presetLoaded(const PresetManager::Preset& preset)
{
    currentPresetLabel.setText(preset.name + " (" + preset.category + ")", juce::dontSendNotification);
    presetListBox.repaint();
}

void PresetBrowserComponent::presetSaved(const PresetManager::Preset& preset)
{
    updatePresetList();
    // Reset category filter to show all presets
    categorySelector.setSelectedId(1, juce::sendNotification);
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

        // Reset category filter to show all presets
        categorySelector.setSelectedId(1, juce::sendNotification);

        // Update selection in the list - runs after updatePresetList via onChange
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
    else
    {
        // No preset loaded - reset to default state
        currentPresetLabel.setText("No Preset Loaded", juce::dontSendNotification);
        presetListBox.deselectAllRows();
    }
}

void PresetBrowserComponent::timerCallback()
{
    // Enable/disable based on state
    auto* currentPreset = presetManager.getCurrentPreset();
    deleteButton.setEnabled(currentPreset != nullptr);
    midiMapButton.setEnabled(currentPreset != nullptr);

    preserveMidiChannelButton.setToggleState(
        presetManager.getPreserveMidiChannel(),
        juce::dontSendNotification);
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
    window->addTextEditor("category", "", "Category:");

    // Populate a hint below the category field showing existing categories
    auto categories = presetManager.getCategories();
    juce::String categoryHint = "Existing: ";
    if (categories.isEmpty())
        categoryHint += "none";
    else
        categoryHint += categories.joinIntoString(", ");

    window->addTextBlock(categoryHint);

    window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            if (result == 1)
            {
                auto presetName = window->getTextEditorContents("presetName");
                auto category = window->getTextEditorContents("category");

                if (category.isEmpty())
                    category = "User"; // Sensible default

                if (presetName.isNotEmpty())
                {
                    presetManager.savePreset(presetName, category);
                }

                // Reset category filter to show all presets
                categorySelector.setSelectedId(1, juce::sendNotification);
            }
        }), true);
}

void PresetBrowserComponent::showDeletePresetDialog()
{
    auto* currentPreset = presetManager.getCurrentPreset();
    if (!currentPreset)
        return;

    // Copy the file before any modal dialog, since the pointer
    // into the presets array can be invalidated during the blocking call
    juce::File fileToDelete = currentPreset->file;

    auto* window = new juce::AlertWindow(
        "Delete Preset",
        "Are you sure you want to delete '" + fileToDelete.getFileNameWithoutExtension() + "'?",
        juce::AlertWindow::WarningIcon
    );

    window->addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, fileToDelete](int result)
        {
            if (result == 1)
            {
                presetManager.deletePreset(fileToDelete);

                // Reset category filter to show all presets
                categorySelector.setSelectedId(1, juce::sendNotification);
            }
        }), true);
}

void PresetBrowserComponent::showMidiMappingDialog()
{
    juce::File presetFile;
    {
        auto* currentPreset = presetManager.getCurrentPreset();
        if (!currentPreset)
            return;
        presetFile = currentPreset->file;
    }

    if (!presetFile.exists()) // for safety
        return;

    auto currentNote = presetManager.getMidiNoteForPreset(presetFile);

    // Create modal window
    auto* dialog = new MidiLearnDialog(presetManager, presetFile, currentNote);

    juce::Component::SafePointer<MidiLearnDialog> safeDialog(dialog);

    // Callbacks
    presetMidiHandler.setMidiLearnCallback([safeDialog](int note) {
        juce::MessageManager::callAsync([safeDialog, note]() {
            if (safeDialog != nullptr)
                safeDialog->midiNoteReceived(note);
            });
        });

    // Handle dialog close
    dialog->onClose = [this]() {
        presetMidiHandler.clearMidiLearnCallback();

        // update preset list
        juce::MessageManager::callAsync([this]() {
            updatePresetList();
            });
        };

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(dialog);
    options.dialogTitle = "MIDI Mapping";
    options.dialogBackgroundColour = juce::Colour(0xff2a2a2a);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.useBottomRightCornerResizer = false;

    options.launchAsync();
}

void PresetBrowserComponent::updatePreserveMidiChannelButton()
{
    bool isPreserving = presetManager.getPreserveMidiChannel();

    if (isPreserving)
    {
        preserveMidiChannelButton.setColour(juce::TextButton::buttonColourId,
            juce::Colours::darkgreen.darker(0.3f));
    }
    else
    {
        preserveMidiChannelButton.setColour(juce::TextButton::buttonColourId,
            juce::Colours::darkred.darker(0.3f));
    }

}