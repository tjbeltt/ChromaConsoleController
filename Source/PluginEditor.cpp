/*
  ==============================================================================

    Refactored PluginEditor.cpp
    Cleaner modular approach with CCSliderModule components

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Color definitions
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::red(235, 78, 40);
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::yellow(230, 205, 36);
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::green(85, 194, 84);
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::blue(105, 210, 228);
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::purple(95, 49, 160);

ChromaConsoleControllerAudioProcessorEditor::ChromaConsoleControllerAudioProcessorEditor(
    ChromaConsoleControllerAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    channelAttachment(p.parameters, "midiChannel", channelSelector),
    updateAttachment(p.parameters, "updateValues", updateButton)
{
    // Setup channel selector
    addAndMakeVisible(channelSelector);
    juce::StringArray channels;
    for (int i = 1; i <= 16; ++i)
        channels.add("Ch " + juce::String(i));
    channelSelector.addItemList(channels, 1);
    channelSelector.setSelectedId(*audioProcessor.parameters.getRawParameterValue("midiChannel"));

    // Setup update button
    addAndMakeVisible(updateButton);
    updateButton.setButtonText("Update All Values");
    updateButton.onClick = [this]() { audioProcessor.sendCurrentSliderValues(); };

    // Setup advanced settings button
    addAndMakeVisible(advancedButton);
    advancedButton.setButtonText(getShowAdvancedSettings() ? "Hide Advanced" : "Advanced Settings");
    advancedButton.onClick = [this]() { toggleAdvancedSettings(); };

    // Setup version label
    versionNumber.setText(JucePlugin_VersionString, juce::dontSendNotification);
    versionNumber.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(versionNumber);

    // Create CC modules
    createCCModules();

    // Setup column interactions
    setupColumnInteractions();

    // Set initial size (now more flexible)
    setResizable(true, true);
    setConstrainer(&constrainer);
    constrainer.setSizeLimits(600, 700, 1920, 1080);
    
    // Calculate initial height for basic view
    int initialHeight = calculateIdealHeight();
    setSize(600, initialHeight);

    // Initialize column properties
    if (!ccModules.empty()) {
        setColumnProperties(0, ccModules[0]->getSlider().getValue(), true, true, true, true);
        setColumnProperties(1, ccModules[1]->getSlider().getValue(), true, true, true, true);
        setColumnProperties(2, ccModules[2]->getSlider().getValue(), true, true, true, true);
        if (ccModules.size() > 3) {
            setColumnProperties(3, ccModules[3]->getSlider().getValue(), true, true, false, false);
        }
    }
}

ChromaConsoleControllerAudioProcessorEditor::~ChromaConsoleControllerAudioProcessorEditor()
{
}

void ChromaConsoleControllerAudioProcessorEditor::createCCModules()
{
    const auto& configs = ChromaConsoleControllerAudioProcessor::ccConfigurations;

    for (const auto& config : configs) {
        auto module = std::make_unique<CCSliderModule>(audioProcessor, config);
        addAndMakeVisible(*module);
        ccModules.push_back(std::move(module));
    }
}

void ChromaConsoleControllerAudioProcessorEditor::setupColumnInteractions()
{
    // Setup value change callbacks for the first 4 modules (column headers)
    if (ccModules.size() >= 4) {
        ccModules[0]->getSlider().onValueChange = [this]() {
            setColumnProperties(0, ccModules[0]->getSlider().getValue(), true, true, true, true);
            };

        ccModules[1]->getSlider().onValueChange = [this]() {
            setColumnProperties(1, ccModules[1]->getSlider().getValue(), true, true, true, true);
            };

        ccModules[2]->getSlider().onValueChange = [this]() {
            setColumnProperties(2, ccModules[2]->getSlider().getValue(), true, true, true, true);
            };

        ccModules[3]->getSlider().onValueChange = [this]() {
            setColumnProperties(3, ccModules[3]->getSlider().getValue(), true, true, false, false);
            };
    }
}

void ChromaConsoleControllerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ChromaConsoleControllerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(padding);

    // Header area
    auto headerArea = area.removeFromTop(headerHeight);
    channelSelector.setBounds(headerArea.removeFromRight(150));
    updateButton.setBounds(headerArea.removeFromLeft(150));
    versionNumber.setBounds(headerArea);

    // Footer area for advanced button
    auto footerArea = area.removeFromBottom(footerHeight);
    advancedButton.setBounds(footerArea.withSizeKeepingCentre(150, 25));

    // Calculate grid layout
    const int visibleRows = calculateVisibleRows();
    if (visibleRows == 0) return;

    auto gridArea = area;
    const int cellWidth = gridArea.getWidth() / numColumns;
    const int cellHeight = gridArea.getHeight() / visibleRows;

    // Position modules in grid - only show visible ones
    int visibleModuleCount = visibleRows * numColumns;
    for (int i = 0; i < std::min(visibleModuleCount, (int)ccModules.size()); ++i) {
        const int row = i / numColumns;
        const int col = i % numColumns;

        int xPos = gridArea.getX() + col * cellWidth;
        int yPos = gridArea.getY() + row * cellHeight;

        // Add small padding between modules
        const int modulePadding = 5;
        ccModules[i]->setBounds(xPos + modulePadding, yPos + modulePadding,
            cellWidth - 2 * modulePadding, cellHeight - 2 * modulePadding);
        ccModules[i]->setVisible(true);
    }

    // Hide modules that should not be visible
    for (int i = visibleModuleCount; i < ccModules.size(); ++i) {
        ccModules[i]->setVisible(getShowAdvancedSettings());
    }
}

int ChromaConsoleControllerAudioProcessorEditor::calculateVisibleRows() const
{
    if (ccModules.empty()) return 0;

    if (getShowAdvancedSettings()) {
        // Show all rows when advanced settings are visible
        return calculateNumRows();
    }
    else {
        // Show only basic rows (first 3 rows)
        return std::min(basicRows, calculateNumRows());
    }
}

int ChromaConsoleControllerAudioProcessorEditor::calculateIdealHeight() const
{
    const int visibleRows = calculateVisibleRows();
    const int cellHeight = 120; // Approximate height per cell

    int idealHeight = headerHeight + footerHeight + (visibleRows * cellHeight) + (2 * padding);

    // Ensure we don't go below the minimum height restriction
    idealHeight = std::max(idealHeight, constrainer.getMinimumHeight());
    
    return idealHeight;
}

void ChromaConsoleControllerAudioProcessorEditor::toggleAdvancedSettings()
{
    bool currentState = getShowAdvancedSettings();
    bool newState = !currentState;
    setShowAdvancedSettings(newState);

    // Update button text
    advancedButton.setButtonText(newState ? "Hide Advanced" : "Advanced Settings");

    // Calculate new ideal height
    int newHeight = calculateIdealHeight();

    // Animate the resize
    juce::ComponentAnimator& animator = juce::Desktop::getInstance().getAnimator();
    animator.animateComponent(this,
        getBounds().withHeight(newHeight),
        1.0f, 200, false, 1.0, 0.0);

    // Update visibility of advanced modules immediately for smooth animation
    int visibleModuleCount = calculateVisibleRows() * numColumns;
    for (int i = visibleModuleCount; i < ccModules.size(); ++i) {
        ccModules[i]->setVisible(newState);
    }
}

int ChromaConsoleControllerAudioProcessorEditor::calculateNumRows() const
{
    if (ccModules.empty()) return 0;
    return (ccModules.size() + numColumns - 1) / numColumns;
}

void ChromaConsoleControllerAudioProcessorEditor::setColumnProperties(int column, int value,
    bool first, bool second, bool third, bool fourth)
{
    // Enable/disable column
    bool enabled = (value != 5);
    setColumnEnabled(column, enabled, first, second, third, fourth);

    // Set column color
    juce::Colour colour = getColourForValue(value);
    setColumnColour(column, colour, first, second, third, fourth);
}

void ChromaConsoleControllerAudioProcessorEditor::setColumnEnabled(int column, bool enabled,
    bool first, bool second, bool third, bool fourth)
{
    int offset = column % 4;

    if (first && (4 + offset) < ccModules.size()) {
        ccModules[4 + offset]->setEnabled(enabled);
    }
    if (second && (8 + offset) < ccModules.size()) {
        ccModules[8 + offset]->setEnabled(enabled);
    }
    if (third && (12 + offset) < ccModules.size()) {
        ccModules[12 + offset]->setEnabled(enabled);
    }
    if (fourth && (16 + offset) < ccModules.size()) {
        ccModules[16 + offset]->setEnabled(enabled);
    }
}

void ChromaConsoleControllerAudioProcessorEditor::setColumnColour(int column, juce::Colour colour,
    bool first, bool second, bool third, bool fourth)
{
    int offset = column % 4;

    // Set color for header module
    if (offset < ccModules.size()) {
        ccModules[offset]->setSliderColours(sliderColourIds, colour);
    }

    if (first && (4 + offset) < ccModules.size()) {
        ccModules[4 + offset]->setSliderColours(sliderColourIds, colour);
    }
    if (second && (8 + offset) < ccModules.size()) {
        ccModules[8 + offset]->setSliderColours(sliderColourIds, colour);
    }
    if (third && (12 + offset) < ccModules.size()) {
        ccModules[12 + offset]->setSliderColours(sliderColourIds, colour);
    }
    if (fourth && (16 + offset) < ccModules.size()) {
        ccModules[16 + offset]->setSliderColours(sliderColourIds, colour);
    }
}

juce::Colour ChromaConsoleControllerAudioProcessorEditor::getColourForValue(int value) const
{
    switch (value) {
    case 0: return Colors::red;
    case 1: return Colors::yellow;
    case 2: return Colors::green;
    case 3: return Colors::blue;
    case 4: return Colors::purple;
    default: return getLookAndFeel().findColour(juce::Slider::ColourIds::backgroundColourId);
    }
}

bool ChromaConsoleControllerAudioProcessorEditor::getShowAdvancedSettings() const
{
    // Get the property from the processor's parameter state
    auto* property = audioProcessor.parameters.state.getPropertyPointer("showAdvancedSettings");
    return property != nullptr ? (bool)*property : false;
}

void ChromaConsoleControllerAudioProcessorEditor::setShowAdvancedSettings(bool show)
{
    // Store the property in the processor's parameter state
    audioProcessor.parameters.state.setProperty("showAdvancedSettings", show, nullptr);
}