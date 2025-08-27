/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class GreendistortionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    GreendistortionAudioProcessorEditor (GreendistortionAudioProcessor&);
    ~GreendistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GreendistortionAudioProcessor& audioProcessor;

    juce::Slider driveSlider, toneSlider, highpassSlider, mixSlider, outputSlider;

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> toneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highpassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
	


	juce::Label driveLabel, toneLabel, highpassLabel, mixLabel, outputLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GreendistortionAudioProcessorEditor)
};
