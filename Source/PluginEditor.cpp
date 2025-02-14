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

    // Create CC controls
    const auto& configs = ChromaConsoleControllerAudioProcessor::ccConfigurations;
    for (const auto& config : configs) {
        createCCControl(config);
    }

    
    setResizable(true, true);
    setSize(950, 900);
    repaint();
}

ChromaConsoleControllerAudioProcessorEditor::~ChromaConsoleControllerAudioProcessorEditor()
{
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
