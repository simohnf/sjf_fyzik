/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_karplusStrongAudioProcessor::Sjf_karplusStrongAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    for ( auto& string : m_strings )
        string.prepare( getSampleRate() );
    
    for ( auto& block : m_dcBlock )
        block.setCutoff( calculateLPFCoefficient< double >( 30, getSampleRate() ) );
}

Sjf_karplusStrongAudioProcessor::~Sjf_karplusStrongAudioProcessor()
{
}

//==============================================================================
const juce::String Sjf_karplusStrongAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sjf_karplusStrongAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_karplusStrongAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_karplusStrongAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sjf_karplusStrongAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sjf_karplusStrongAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sjf_karplusStrongAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sjf_karplusStrongAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sjf_karplusStrongAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sjf_karplusStrongAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sjf_karplusStrongAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for ( auto& string : m_strings )
        string.prepare( sampleRate );
    for ( auto& block : m_dcBlock )
        block.setCutoff( calculateLPFCoefficient< double >( 30, sampleRate ) );
}

void Sjf_karplusStrongAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sjf_karplusStrongAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Sjf_karplusStrongAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto bufferSize = buffer.getNumSamples();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

//    for ( auto index = 0; index < bufferSize; index++ )
//    {
//        if( midiMessages )
//    }
//    for ( auto& string : m_strings )
//        string.setAttackBrightness( 100 );
    
    auto noteIndex = 0;
    auto checkMidi = false;
    std::vector< std::array< int, 3 > > noteList;
    auto listIndex = 0;
    for ( const auto event : midiMessages )
    {
        if ( event.getMessage().isNoteOnOrOff() )
        {
            auto pos = event.samplePosition;
            auto pitch = event.getMessage().getNoteNumber();
            auto vel = event.getMessage().isNoteOff() ? 0 : static_cast< int >(event.getMessage().getVelocity());
            noteList.push_back( { pos, pitch, vel } );
        }
    }
    if ( noteList.size() > 0 )
    {
        checkMidi = true;
        noteIndex = noteList[ listIndex ][ 0 ];
    }
    
    for ( auto indexThroughCurrentBuffer = 0; indexThroughCurrentBuffer < bufferSize; indexThroughCurrentBuffer++ )
    {
        while ( checkMidi && ( indexThroughCurrentBuffer == noteIndex ) && ( listIndex < noteList.size() ) )
        {
            if ( noteList[ listIndex ][ 2 ] == 0  )
            {
                if ( m_triggerNoteOffs )
                {
                    for ( auto& str : m_strings )
                    {
                        if ( str.getCurrentPitch() == noteList[ listIndex ][ 1 ] )
                        {
                            str.triggerNoteOff();
                            DBG( "noteoff");
                        }
                    }
                }
            }
            else
            {
                DBG( "noteon");
                DBG(" voice " << m_voiceNum );
                m_strings[ m_voiceNum ].triggerNewNote( noteList[ listIndex ][ 1 ], noteList[ listIndex ][ 2 ] );
                m_voiceNum = fastMod( ++m_voiceNum, m_strings.size() );
            }
            listIndex += 1;
            if( listIndex < noteList.size() )
                noteIndex = noteList[ listIndex ][ 0 ];
            DBG("");
        }
        
        auto val = 0.0;
        for ( auto& string : m_strings )
        {
            val += string.processSample( indexThroughCurrentBuffer );;
        }
        for ( auto channel = 0; channel < totalNumOutputChannels; channel++ )
            buffer.setSample( channel , indexThroughCurrentBuffer, val/*m_dcBlock[ channel ].filterInputHP( val ) */ );
    }

    
    
}

//==============================================================================
bool Sjf_karplusStrongAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sjf_karplusStrongAudioProcessor::createEditor()
{
    return new Sjf_karplusStrongAudioProcessorEditor (*this);
}

//==============================================================================
void Sjf_karplusStrongAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Sjf_karplusStrongAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_karplusStrongAudioProcessor();
}
