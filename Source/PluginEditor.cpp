/*
  ==============================================================================

    Refactored PluginEditor.cpp
    Cleaner modular approach with CCSliderModule components

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Color definitions
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::red(juce::Colour::fromHSL(0.0f, 0.85f, 0.55f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::yellow(juce::Colour::fromHSL(0.13f, 0.90f, 0.55f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::green(juce::Colour::fromHSL(0.33f, 0.75f, 0.48f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::blue(105, 210, 228);
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::purple(95, 49, 160);
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::bg_dark(juce::Colour::fromHSL((                   0.0f), 0.00f, .04f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::bg(juce::Colour::fromHSL((                        0.0f), 0.00f, .10f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::bg_light(juce::Colour::fromHSL((                  0.0f), 0.00f, .15f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::text(juce::Colour::fromHSL((                      0.0f), 0.00f, .88f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::text_muted(juce::Colour::fromHSL((                0.0f), 0.00f, .38f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::highlight(juce::Colour::fromHSL((       38.0f / 360.0f), 0.03f, .47f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::border(juce::Colour::fromHSL((          26.0f / 360.0f), 0.04f, .63f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::border_muted(juce::Colour::fromHSL((              0.0f), 0.00f, .22f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::primary(juce::Colour::fromHSL((         03.0f / 360.0f), 0.46f, .50f, 1.0f));
const juce::Colour ChromaConsoleControllerAudioProcessorEditor::Colors::secondary(juce::Colour::fromHSL((      192.0f / 360.0f), 0.99f, .58f, 1.0f));



ChromaConsoleControllerAudioProcessorEditor::ChromaConsoleControllerAudioProcessorEditor(
    ChromaConsoleControllerAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    channelAttachment(p.parameters, "midiChannel", channelSelector),
    updateAttachment(p.parameters, "updateValues", updateButton),
    presetBrowser(p.getPresetManager(), p.getPresetMidiHandler())
{    
    setLookAndFeel(&lnf);

    // Preset Browser =========================
    // Preset Browser button
    addAndMakeVisible(presetButton);
    presetButton.setButtonText("Presets");
    presetButton.onClick = [this]() { togglePresetBrowser(); };

    // Add preset browser
    addChildComponent(presetBrowser);
    presetBrowser.setVisible(false);

    // Main Setup ==============================
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
    versionNumber.setColour(juce::Label::outlineColourId, juce::Colour(0.f, 0.f, 0.f, 0.f));
    addAndMakeVisible(versionNumber);

    // Create CC modules
    createCCModules();

    // Setup column interactions
    setupColumnInteractions();

    // Set initial size
    setResizable(true, false);
    setConstrainer(&constrainer);
    constrainer.setSizeLimits(600, 700, 1920, 1080);
    
    // Calculate initial height for basic view
    int initialHeight = calculateIdealHeight();
    setSize(600, initialHeight);

    setLAF();

    // Initialize column properties
    if (!ccModules.empty()) {
        setColumnProperties(0, ccModules[0]->getSlider().getValue(), true, true, true, true);
        setColumnProperties(1, ccModules[1]->getSlider().getValue(), true, true, true, true);
        setColumnProperties(2, ccModules[2]->getSlider().getValue(), true, true, true, true);
        setColumnProperties(3, ccModules[3]->getSlider().getValue(), true, true, false, false);
    }

    setOpaque(true);
}

ChromaConsoleControllerAudioProcessorEditor::~ChromaConsoleControllerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void ChromaConsoleControllerAudioProcessorEditor::setLAF()
{
    auto& laf = getLookAndFeel();

    //BG
    laf.setColour(juce::ResizableWindow::backgroundColourId, Colors::bg);

    //Text Button
    laf.setColour(juce::TextButton::ColourIds::buttonColourId, Colors::bg_light);
    laf.setColour(juce::TextButton::ColourIds::buttonOnColourId, Colors::bg_light.brighter(.2f));
    laf.setColour(juce::TextButton::ColourIds::textColourOnId, Colors::text);
    laf.setColour(juce::TextButton::ColourIds::textColourOffId, Colors::text);

    //Popup Menu
    laf.setColour(juce::PopupMenu::ColourIds::textColourId, Colors::text_muted);
    laf.setColour(juce::PopupMenu::ColourIds::headerTextColourId, Colors::text);
    laf.setColour(juce::PopupMenu::ColourIds::backgroundColourId, Colors::bg_dark);
    laf.setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, Colors::text.brighter(.2f));
    laf.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, Colors::bg_light);

    //Label
    laf.setColour(juce::Label::textColourId, Colors::text);
    laf.setColour(juce::Label::outlineColourId, Colors::border_muted);
    
    //Slider
    laf.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Colors::text);
    laf.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colour(0.0f, 0.0f, 0.0f, 0.0f));
    laf.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, Colors::bg_light);
    laf.setColour(juce::Slider::ColourIds::textBoxTextColourId, Colors::text);
    laf.setColour(juce::Slider::ColourIds::textBoxHighlightColourId, Colors::highlight);
    laf.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, Colors::border_muted);
    laf.setColour(juce::Slider::ColourIds::thumbColourId, Colors::bg_light);
    laf.setColour(CoveLNF::CoveRotarySlider::thumbEnabledID, Colors::text);
    laf.setColour(CoveLNF::CoveRotarySlider::thumbDisabledID, Colors::text_muted);
    laf.setColour(CoveLNF::CoveRotarySlider::backgroundArcID, Colors::bg_dark);

    //ComboBox
    laf.setColour(juce::ComboBox::backgroundColourId, Colors::bg_light);
    laf.setColour(juce::ComboBox::textColourId, Colors::text);
    laf.setColour(juce::ComboBox::outlineColourId, Colors::border_muted);
    laf.setColour(juce::ComboBox::arrowColourId, Colors::text_muted);
    laf.setColour(juce::ComboBox::focusedOutlineColourId, Colors::border);

    for (auto& module : ccModules)
    {
        module->setSliderColour(juce::Slider::textBoxTextColourId, Colors::text);
        module->setSliderColour(juce::Slider::textBoxBackgroundColourId, Colors::bg_light);
        module->setSliderColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0.f, 0.f, 0.f, 0.f));
    }
    
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
    //setLAF();
}

void ChromaConsoleControllerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(padding);

    // If preset browser visible, reserve space
    if (showPresetBrowser)
    {
        auto presetArea = area.removeFromLeft(presetBrowserWidth);
        presetBrowser.setBounds(presetArea);
        area.removeFromLeft(padding);
    }
    
    // Header area
    auto headerArea = area.removeFromTop(headerHeight);
    channelSelector.setBounds(headerArea.removeFromRight(150));
    updateButton.setBounds(headerArea.removeFromLeft(150));
    headerArea.removeFromLeft(5);
    presetButton.setBounds(headerArea.removeFromLeft(100));
    versionNumber.setBounds(headerArea);

    // Footer area for advanced button
    auto footerArea = area.removeFromBottom(footerHeight);
    advancedButton.setBounds(footerArea.withSizeKeepingCentre(150, 25));

    // Calculate grid layout
    const int visibleRows = calculateVisibleRows();
    if (visibleRows == 0) return;

    auto gridArea = area;
    const int cellWidth = gridArea.getWidth() / numColumns;

    // Use stable cell height derived from ideal layout,
    // not current animated height. This prevents erratic slider growth.
    // This breaks scaling of sliders, though it might be a good trade-off
    //const int cellHeight = gridArea.getHeight() / visibleRows;
    const int idealGridHeight = (visibleRows * 120);
    const int cellHeight = idealGridHeight / visibleRows;

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
    bool enabled = (value < 5);
    setColumnEnabled(column, enabled, first, second, third, fourth);

    if (enabled)
    { // Set column color
        juce::Colour colour = getColourForValue(value);
        setColumnColour(column, colour, first, second, third, fourth);
        setColumnTextBoxColour(column, Colors::text, Colors::bg_light, juce::Colour(0.f, 0.f, 0.f, 0.f), first, second, third, fourth);
    }
    else {
        setColumnColour(column, Colors::text_muted, first, second, third, fourth);
        setColumnTextBoxColour(column, Colors::text_muted, Colors::bg_dark, juce::Colour(0.f, 0.f, 0.f, 0.f), first, second, third, fourth);
    }
    
    // Override header slider's thumb color to show that they are active
    int offset = column % 4;
    if (offset < (int)ccModules.size())
    {
        ccModules[offset]->setSliderColour(CoveLNF::CoveRotarySlider::thumbEnabledID, Colors::text);
    }
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

void ChromaConsoleControllerAudioProcessorEditor::setColumnTextBoxColour(int column, juce::Colour textColour, juce::Colour bgColour, juce::Colour outlineColour,
    bool first, bool second, bool third, bool fourth)
{
    int offset = column % 4;
    auto applyToModule = [&](int index) {
        if (index < ccModules.size())
        {
            ccModules[index]->setSliderColour(juce::Slider::textBoxTextColourId, textColour);
            ccModules[index]->setSliderColour(juce::Slider::textBoxBackgroundColourId, bgColour);
            ccModules[index]->setSliderColour(juce::Slider::textBoxOutlineColourId, outlineColour);
        }
    };

    applyToModule(offset);
    if (first)  applyToModule(4 + offset);
    if (second) applyToModule(8 + offset);
    if (third)  applyToModule(12 + offset);
    if (fourth) applyToModule(16 + offset);
}

juce::Colour ChromaConsoleControllerAudioProcessorEditor::getColourForValue(int value) const
{
    switch (value) {
    case 0: return Colors::red;
    case 1: return Colors::yellow;
    case 2: return Colors::green;
    case 3: return Colors::blue;
    case 4: return Colors::purple;
    default: return getLookAndFeel().findColour(CoveLNF::CoveRotarySlider::backgroundArcID);
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

void ChromaConsoleControllerAudioProcessorEditor::togglePresetBrowser()
{
    showPresetBrowser = !showPresetBrowser;
    presetBrowser.setVisible(showPresetBrowser);

    // Update button text
    presetButton.setButtonText(showPresetBrowser ? "Hide Presets" : "Presets");

    // Animate the resize to show/hide the preset browser
    int newWidth = getWidth();
    if (showPresetBrowser)
        newWidth += presetBrowserWidth + padding;
    else
        newWidth -= presetBrowserWidth + padding;

    // Ensure we stay within constraints
    newWidth = juce::jlimit(constrainer.getMinimumWidth(),
        constrainer.getMaximumWidth(),
        newWidth);

    juce::ComponentAnimator& animator = juce::Desktop::getInstance().getAnimator();
    animator.animateComponent(this,
        getBounds().withWidth(newWidth),
        1.0f, 200, false, 1.0, 0.0);
}