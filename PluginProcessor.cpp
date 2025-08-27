  /*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GreendistortionAudioProcessor::GreendistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
             apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout GreendistortionAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	layout.add(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 1.0f, 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("tone", "Lowpass", 20.0f, 20000.0f, 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("highpass", "Highpass", 20.0f, 20000.0f, 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("output", "Output", -24.0f, 24.0f, 0.0f));  
	return layout;
}

GreendistortionAudioProcessor::~GreendistortionAudioProcessor()
{
}

//==============================================================================
const juce::String GreendistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GreendistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GreendistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GreendistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GreendistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GreendistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GreendistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GreendistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GreendistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void GreendistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GreendistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(samplesPerBlock);

    lowpassFilterLeft.reset();
    lowpassFilterRight.reset();

	float defaultCutoff = 20.0f;
	auto coeffs = juce::IIRCoefficients::makeLowPass(sampleRate, defaultCutoff);

	lowpassFilterLeft.setCoefficients(coeffs); 
	lowpassFilterRight.setCoefficients(coeffs);   

	auto highpass = juce::IIRCoefficients::makeHighPass(sampleRate, 20.0f);
	highpassFilterLeft.setCoefficients(highpass);
    highpassFilterRight.setCoefficients(highpass);

}

void GreendistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GreendistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GreendistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)   
        buffer.clear (i, 0, buffer.getNumSamples());

	float drive = apvts.getRawParameterValue("drive")->load();
    float tone = apvts.getRawParameterValue("tone")->load();
    float mix = apvts.getRawParameterValue("mix")->load();
    float output = apvts.getRawParameterValue("output")->load();
	float highpassCutoff = apvts.getRawParameterValue("highpass")->load();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
   
    juce::AudioBuffer<float>dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* samples = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float clean = dryBuffer.getReadPointer(channel)[i];
			float distorted = std::tanh(drive * clean * 12.0f);
            samples[i] = distorted;
        }

        // ..do something to the data...
    }

    //stops user automation going past the limits
    float cutoff = std::clamp(tone, 20.0f, 20000.0f);

    if (std::abs(cutoff - lastCutoff) > 1.0f)
    {
        auto coeffs = juce::IIRCoefficients::makeLowPass(getSampleRate(), cutoff);
        lowpassFilterLeft.setCoefficients(coeffs);
        lowpassFilterRight.setCoefficients(coeffs);
        lastCutoff = cutoff;
    }

    lowpassFilterLeft.processSamples(buffer.getWritePointer(0), buffer.getNumSamples());
    if (buffer.getNumChannels() > 1)
        lowpassFilterRight.processSamples(buffer.getWritePointer(1), buffer.getNumSamples());

	if (std::abs(highpassCutoff - lastHighpassCutoff) > 1.0f)
	{
		auto highpassCoeffs = juce::IIRCoefficients::makeHighPass(getSampleRate(), highpassCutoff);
		highpassFilterLeft.setCoefficients(highpassCoeffs);
		highpassFilterRight.setCoefficients(highpassCoeffs);
		lastHighpassCutoff = highpassCutoff;
	}

    highpassFilterLeft.processSamples(buffer.getWritePointer(0), buffer.getNumSamples());
    if (buffer.getNumChannels() > 1)
        highpassFilterRight.processSamples(buffer.getWritePointer(1), buffer.getNumSamples());

    
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
		auto* wet = buffer.getWritePointer(channel);
		auto* dry = dryBuffer.getReadPointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            wet[i] = dry[i] * (1.0f - mix) + wet[i] * mix;
		}
        
    }   
    float outputGain = std::pow(10.0f, output / 20.0f);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        buffer.applyGain(channel, 0, buffer.getNumSamples(), outputGain);
   
}

//==============================================================================
bool GreendistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GreendistortionAudioProcessor::createEditor()
{
    return new GreendistortionAudioProcessorEditor (*this);
}

//==============================================================================
void GreendistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GreendistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GreendistortionAudioProcessor();
}
