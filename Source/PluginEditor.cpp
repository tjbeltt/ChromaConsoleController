/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChromaConsoleControllerAudioProcessorEditor::ChromaConsoleControllerAudioProcessorEditor (ChromaConsoleControllerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), 
    channelAttachment(p.parameters, "midiChannel", channelSelector), 
    updateAttachment(p.parameters, "updateValues", updateButton)
{

    

    // Channel selector setup
    addAndMakeVisible(channelSelector);
    juce::StringArray channels;
    for (int i = 1; i <= 16; ++i)
        channels.add("Ch " + juce::String(i));
    channelSelector.addItemList(channels, 1);
    channelSelector.setSelectedId(*audioProcessor.parameters.getRawParameterValue("midiChannel"));

    // Update button setup
    addAndMakeVisible(updateButton);
    updateButton.setButtonText("Update All Values");
    updateButton.onClick = [this]() { audioProcessor.sendCurrentSliderValues(); };

    versionNumber.setText(JucePlugin_VersionString, juce::dontSendNotification);
    versionNumber.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(versionNumber);

    // Create CC controls
    const auto& configs = ChromaConsoleControllerAudioProcessor::ccConfigurations;
    for (const auto& config : configs) {
        createCCControl(config);
    }

    runColumnOVC(0, true, true, true, true);
    runColumnOVC(1, true, true, true, true);
    runColumnOVC(2, true, true, true, true);
    runColumnOVC(3, true, true, false, false);

    
    

    
    setResizable(false, false);
    setSize(950, 900);
    repaint();

    setColumnProperties(0, ccSliders[0].get()->getValue(), true, true, true, true);
    setColumnProperties(1, ccSliders[1].get()->getValue(), true, true, true, true);
    setColumnProperties(2, ccSliders[2].get()->getValue(), true, true, true, true);
    setColumnProperties(3, ccSliders[3].get()->getValue(), true, true, false, false);
}

ChromaConsoleControllerAudioProcessorEditor::~ChromaConsoleControllerAudioProcessorEditor()
{
}

void ChromaConsoleControllerAudioProcessorEditor::runColumnOVC(int offset, bool first, bool second, bool third, bool fourth)
{
    //auto value = ccSliders[0 + offset]->getValue();
    ccSliders[0 + (offset % 4)]->onValueChange = [this, offset, first, second, third, fourth] { // onValueChange for 'parent' slider in column
        setColumnProperties(offset, ccSliders[0 + (offset % 4)].get()->getValue(), first, second, third, fourth); // setColumnProperties. 
        //If any of the first four are disabled, dont run any properties for them. Get the value of the top value slider and pass it into the
        //function as 'value'
    };
}

void ChromaConsoleControllerAudioProcessorEditor::setColumnProperties(int offset, int value, bool first, bool second, bool third, bool fourth)
{
    juce::Colour red = juce::Colour(235, 78, 40);
    juce::Colour yellow = juce::Colour(230, 205, 36);
    juce::Colour green = juce::Colour(85, 194, 84);
    juce::Colour blue = juce::Colour(105, 210, 228);
    juce::Colour purple = juce::Colour(95, 49, 160);

    // Check if sliders in column should be enabled or disabled
    if (value == 5) { // If Slider is at max value
        setColumnEnabled(offset, false, first, second, third, fourth);
    }
    else {
        setColumnEnabled(offset, true, first, second, third, fourth);
    }

    //Set column colour based off of value (parent slider position)
    if (value == 0) { setColumnColour(sliderColourIds, red, offset, first, second, third, fourth); }
    else if (value == 1) { setColumnColour(sliderColourIds, yellow, offset, first, second, third, fourth); }
    else if (value == 2) { setColumnColour(sliderColourIds, green, offset, first, second, third, fourth); }
    else if (value == 3) { setColumnColour(sliderColourIds, blue , offset, first, second, third, fourth); }
    else if (value == 4) { setColumnColour(sliderColourIds, purple, offset, first, second, third, fourth); }
    else { setColumnColour(sliderColourIds, getLookAndFeel().findColour(juce::Slider::ColourIds::backgroundColourId), offset, first, second, third, fourth); }

    
}

// Set the column enabled state
void ChromaConsoleControllerAudioProcessorEditor::setColumnEnabled(int column, bool enabled, bool first, bool second, bool third, bool fourth) {
    int offset = column % 4;
    if (first) {
        ccSliders[4 + offset]->setEnabled(enabled);
    }
    if (second) {
        ccSliders[8 + offset]->setEnabled(enabled);
    }
    if (third) {
        ccSliders[12 + offset]->setEnabled(enabled);
    }
    if (fourth) {
        ccSliders[16 + offset]->setEnabled(enabled);
    }
}

// Set Coloumn Colour with single colourID enum sent
void ChromaConsoleControllerAudioProcessorEditor::setColumnColour(int colourID, juce::Colour colour, int column, bool first, bool second, bool third, bool fourth) {
    int offset = column % 4;
    if (first) {
        ccSliders[4 + offset]->setColour(colourID, colour);
    }
    if (second) {
        ccSliders[8 + offset]->setColour(colourID, colour);
    }
    if (third) {
        ccSliders[12 + offset]->setColour(colourID, colour);
    }
    if (fourth) {
        ccSliders[16 + offset]->setColour(colourID, colour);
    }
}

// Set Coloumn Colour with vector of colour IDs sent.
void ChromaConsoleControllerAudioProcessorEditor::setColumnColour(std::vector<int> colourIds, juce::Colour colour, int column, bool first, bool second, bool third, bool fourth) {
    int offset = column % 4;
    for (int i = 0; i < colourIds.size(); i++) {
        //Sanity
        ccSliders[0 + offset]->setColour(colourIds[i], colour);

        if (first) {
            ccSliders[4 + offset]->setColour(colourIds[i], colour);
        }
        if (second) {
            ccSliders[8 + offset]->setColour(colourIds[i], colour);
        }
        if (third) {
            ccSliders[12 + offset]->setColour(colourIds[i], colour);
        }
        if (fourth) {
            ccSliders[16 + offset]->setColour(colourIds[i], colour);
        }
    }
    
}

void ChromaConsoleControllerAudioProcessorEditor::createCCControl(const CCControllerConfig& config)
{
    // Create and configure slider
    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider->setRange(0, 127, 1);
    slider->setEnabled(true);
    addAndMakeVisible(*slider);
    ccSliders.push_back(std::move(slider));
    


    // Create and configure label
    auto label = std::make_unique<juce::Label>();
    label->setText(config.name, juce::dontSendNotification);
    label->setFont(12);
    label->setJustificationType(juce::Justification::centred);
    label->setEnabled(true);
    label->setInterceptsMouseClicks(false, false);
    addAndMakeVisible(*label);
    ccLabels.push_back(std::move(label));

    // Create attachment
    ccAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters,
        config.parameterID,
        *ccSliders.back()
    ));

}
//==============================================================================
void ChromaConsoleControllerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ChromaConsoleControllerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // Channel selector at top
    auto topArea = area.removeFromTop(25);
    channelSelector.setBounds(topArea.removeFromRight(150));
    updateButton.setBounds(topArea.removeFromLeft(150));
    
    // Currently using the remaining area of the top header as the version number. 
    // If I need to add anything in the future, I will lower the space this section takes
    versionNumber.setBounds(topArea); 

    // Calculate number of rows and columns for the grid
    const int numColumns = 4; // You can adjust this based on your layout needs
    const int numRows = (ccSliders.size() > 0)
        ? ((ccSliders.size() + numColumns - 1) / numColumns)
        : 0;

    // If there are no sliders, return early
    if (numRows == 0)
        return;

    // Calculate cell size
    const int cellWidth = area.getWidth() / numColumns;
    const int cellHeight = area.getHeight() / numRows;

    // Position sliders and labels
    for (size_t i = 0; i < ccSliders.size(); ++i) {
        const int row = i / numColumns;
        const int col = i % numColumns;

        // Calculate the top-left position of each slider based on its row and column
        int xPos = area.getX() + col * cellWidth;
        int yPos = area.getY() + row * cellHeight;

        // Set bounds for the slider and label within their respective cells
        ccSliders[i]->setBounds(xPos, yPos, cellWidth, cellHeight);
        ccSliders[i]->addChildComponent(ccLabels[i].get());
        ccLabels[i]->setBoundsRelative(.35f, .25f, 0.3f, 0.3f);
    }

    repaint();
}
