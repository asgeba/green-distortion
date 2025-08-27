/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GreendistortionAudioProcessorEditor::GreendistortionAudioProcessorEditor (GreendistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 300);

    auto& params = audioProcessor.apvts;

	driveAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(params, "drive", driveSlider));
	toneAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(params, "tone", toneSlider));
	highpassAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(params, "highpass", highpassSlider));
	mixAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(params, "mix", mixSlider));
	outputAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(params, "output", outputSlider));
	

	for (auto& slider : {&driveSlider, &toneSlider, &mixSlider, &outputSlider, &highpassSlider})
	{
		slider->setSliderStyle(juce::Slider::LinearHorizontal);
		slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(*slider);
	}

	driveLabel.setText("Drive", juce::dontSendNotification);
	toneLabel.setText("Tone", juce::dontSendNotification);
	highpassLabel.setText("Highpass", juce::dontSendNotification);
	mixLabel.setText("Mix", juce::dontSendNotification);
	outputLabel.setText("Output", juce::dontSendNotification);
	

	for (auto* label : { &driveLabel, &toneLabel, &highpassLabel, &mixLabel, &outputLabel })
	{
		label->setJustificationType(juce::Justification::centredLeft);
		addAndMakeVisible(*label);
	}
}

GreendistortionAudioProcessorEditor::~GreendistortionAudioProcessorEditor()
{
}

//==============================================================================
void GreendistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::forestgreen);

    
}

void GreendistortionAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	auto area = getLocalBounds().reduced(20);
    auto rowHeight = 40;
	int labelWidth = 60;
    int spacing = 10;

	auto layoutRow = [&](juce::Label& label, juce::Slider& slider)
		{
			auto row = area.removeFromTop(rowHeight);
			label.setBounds(row.removeFromLeft(labelWidth));
			slider.setBounds(row);
			area.removeFromTop(spacing);
		};

	layoutRow(driveLabel, driveSlider);
	layoutRow(toneLabel, toneSlider);
	layoutRow(highpassLabel, highpassSlider);
	layoutRow(mixLabel, mixSlider);
	layoutRow(outputLabel, outputSlider);
	
	

}
