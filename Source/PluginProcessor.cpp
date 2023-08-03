/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_fyzikAudioProcessor::Sjf_fyzikAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, parameters( *this, nullptr, juce::Identifier("sjf_fyzik"), createParameterLayout() )
#endif
{
    for ( auto& string : m_strings )
        string.prepare( getSampleRate() );
    
    for ( auto& block : m_dcBlock )
        block.setCutoff( calculateLPFCoefficient< double >( 30, getSampleRate() ) );
 
//    for ( auto& str : m_pan )
//        str[ 0 ] = str[ 1 ] = std::sqrt( 0.5 );
    
//    for ( auto& pan : m_panSmoothers )
//    {
//        for ( auto& side : pan )
//            side.reset( getSampleRate(), 0.1 );
//    }
    noiseAmountParameter = parameters.getRawParameterValue( "noiseAmount" );
    noiseLPFParameter = parameters.getRawParameterValue( "noiseLPF" );
    exciteExtensionParameter = parameters.getRawParameterValue( "exciteExtension" );
    exciteExponentParameter = parameters.getRawParameterValue( "exciteExponent" );
    exciteCentreParameter = parameters.getRawParameterValue( "exciteCentre" );
    mediumBrightnessParameter = parameters.getRawParameterValue( "brightness" );
    decayParameter = parameters.getRawParameterValue( "decay" );
    sensorParameter = parameters.getRawParameterValue( "sensorPos" );
    stiffnessParameter = parameters.getRawParameterValue( "stiffness" );
    splitParameter = parameters.getRawParameterValue( "split" );
    harmonicParameter = parameters.getRawParameterValue( "harmonic" );
    buzzParameter = parameters.getRawParameterValue( "buzz" );
    buzzOnOffParameter = parameters.getRawParameterValue( "buzzOnOff" );
    jitterParameter = parameters.getRawParameterValue( "jitter" );
    outputLevelParameter = parameters.getRawParameterValue( "output" );
    spreadParameter = parameters.getRawParameterValue( "stereoSpread" );
    
    setParameters();
}

Sjf_fyzikAudioProcessor::~Sjf_fyzikAudioProcessor()
{
}

//==============================================================================
const juce::String Sjf_fyzikAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sjf_fyzikAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_fyzikAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_fyzikAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sjf_fyzikAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sjf_fyzikAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sjf_fyzikAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sjf_fyzikAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sjf_fyzikAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sjf_fyzikAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sjf_fyzikAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for ( auto& string : m_strings )
        string.prepare( sampleRate );
    for ( auto& block : m_dcBlock )
        block.setCutoff( calculateLPFCoefficient< double >( 30, sampleRate ) );
    for ( auto& pan : m_panSmoothers )
    {
        for ( auto& side : pan )
        {
            side.reset( getSampleRate(), 0.1 );
            side.setCurrentAndTargetValue( 0.7071 );
        }
    }
    m_outputSmoother.reset( getSampleRate(), 0.1 );
    m_outputSmoother.setCurrentAndTargetValue( pow( 10.0, m_outputLevel/20.0 ) );
    setParameters();
}

void Sjf_fyzikAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sjf_fyzikAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Sjf_fyzikAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    setParameters();
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto bufferSize = buffer.getNumSamples();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto noteIndex = 0;
    auto checkMidi = false;
    std::vector< std::array< int, 3 > > noteList;
    auto listIndex = 0;
//    double split, stiff, sensorPos, decay, medBright, exciteNoise, exciteEnvC, exciteEnvE, exciteEnvExt, exciteBright, nonLinFactor, harmonic;
    auto split = 0.0, stiff = 0.0, sensorPos = 0.0, decay = 0.0, medBright = 0.0, exciteNoise = 0.0, exciteEnvC = 0.0, exciteEnvE = 0.0, exciteEnvExt = 0.0, exciteBright = 0.0, nonLinFactor = 0.0, harmonic = 0.0, y = 0.0;
    
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
//                        if ( str.getCurrentPitch() == noteList[ listIndex ][ 1 ] )
//                        {
//                            str.triggerNoteOff();
//                            DBG( "noteoff");
//                        }
                    }
                }
            }
            else
            {
                DBG( "noteon");
                DBG(" voice " << m_voiceNum );
                
                if ( m_jitter > 0)
                {
                    DBG("Jit Start");
                    auto jitRange = m_jitter * 0.01;
                    auto jitType = sjf_jitter<float>::limitType::fold;

                    decay = sjf_jitter<double>::addJitter( m_decay, jitRange, 0.1, 20, jitType );
                    split = sjf_jitter<float>::addJitter( m_split, jitRange, 0.1, 0.9, jitType );
                    stiff = sjf_jitter<float>::addJitter( m_stiff, jitRange, -0.9, 0.9, jitType );
                    exciteBright = sjf_jitter<float>::addJitter( m_exciteBright, jitRange, 0, 1, jitType );
                    exciteEnvC = sjf_jitter<float>::addJitter( m_exciteEnvC, jitRange, 0, 1, jitType);
                    exciteEnvE = sjf_jitter<float>::addJitter( m_exciteEnvE, jitRange, 0.01, 50, jitType);
                    exciteNoise = sjf_jitter<float>::addJitter( m_exciteNoise, jitRange, 0, 1, jitType );
                    sensorPos = sjf_jitter<float>::addJitter( m_sensorPos, jitRange, 0.05, 0.95, jitType );
                    
                    medBright = sjf_jitter<float>::addJitter( m_medBright, jitRange, 0, 1, jitType );
                    exciteEnvExt = sjf_jitter<float>::addJitter( m_exciteEnvExt, jitRange, 1, 10, jitType );
                    nonLinFactor = sjf_jitter<float>::addJitter( m_nonLinFactor, jitRange, 0, 0.9, jitType );
                    harmonic = sjf_jitter<float>::addJitter( m_harmonic, jitRange, 0, m_harmonic, jitType );
                    DBG("Jit End");
                }
                else
                {
                    DBG("No Jit Start");
                    decay = m_decay;
                    split = m_split;
                    stiff = m_stiff;
                    exciteEnvC = m_exciteEnvC;
                    exciteEnvE = m_exciteEnvE;
                    exciteNoise = m_exciteNoise;
                    exciteBright = m_exciteBright;
                    sensorPos = m_sensorPos;
                    medBright = m_medBright;
                    exciteEnvExt = m_exciteEnvExt;
                    nonLinFactor = m_nonLinFactor;
                    harmonic = m_harmonic;
                    DBG("No Jit End");
                }
                m_strings[ m_voiceNum ].setNonLinearity( m_nonLinOn );
                m_strings[ m_voiceNum ].triggerNewNote( noteList[ listIndex ][ 1 ], noteList[ listIndex ][ 2 ], split, stiff, sensorPos, 1, decay, medBright, exciteNoise, exciteEnvC, exciteEnvE, exciteEnvExt, exciteBright, nonLinFactor, harmonic, 2 );
                auto pan = ( m_stereoSpread*( rand01() - 0.5 ) ) + 0.5;
                if ( totalNumOutputChannels > 1 )
                {
                    m_panSmoothers[ m_voiceNum ][ 0 ].setTargetValue( std::sqrt( pan ) );
                    m_panSmoothers[ m_voiceNum ][ 1 ].setTargetValue( std::sqrt( 1.0 - pan ) );
                }
                m_voiceNum = fastMod( ++m_voiceNum, m_strings.size() );
            }
            listIndex += 1;
            if( listIndex < noteList.size() )
                noteIndex = noteList[ listIndex ][ 0 ];
            DBG("");
        }
        if ( totalNumOutputChannels > 1 )
        {
            auto val = std::array< double, 2 >{ 0.0, 0.0 };
            for ( auto str = 0; str < NVOICES; str ++ )
            {
                y = m_strings[ str ].processSample( indexThroughCurrentBuffer );
                val [ 0 ] += y * m_panSmoothers[ str ][ 0 ].getNextValue();
                val [ 1 ] += y * m_panSmoothers[ str ][ 1 ].getNextValue();
            }
            for ( auto& v : val )
                v *= m_outputSmoother.getNextValue();
            for ( auto channel = 0; channel < totalNumOutputChannels; channel++ )
                buffer.setSample( channel , indexThroughCurrentBuffer, val[ channel ] );
        }
        else
        {
            y = 0.0;
            for ( auto str = 0; str < NVOICES; str ++ )
            {
                y += m_strings[ str ].processSample( indexThroughCurrentBuffer );
            }
            for ( auto channel = 0; channel < totalNumOutputChannels; channel++ )
                buffer.setSample( channel , indexThroughCurrentBuffer, y );

        }
    }
    
}

//==============================================================================
bool Sjf_fyzikAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sjf_fyzikAudioProcessor::createEditor()
{
    return new Sjf_fyzikAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Sjf_fyzikAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    DBG("GETTING STATE");
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
    DBG("GOT STATE");
}

void Sjf_fyzikAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    DBG("SAVING STATE");
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_fyzikAudioProcessor();
}





juce::AudioProcessorValueTreeState::ParameterLayout Sjf_fyzikAudioProcessor::createParameterLayout()
{
    
    juce::NormalisableRange< float > exponentRange( 0.1, 20, 0.01);
    exponentRange.setSkewForCentre( 2 );
    
    juce::NormalisableRange< float > extensionRange( 1, 50, 0.001);
    extensionRange.setSkewForCentre( 3 );
    
    juce::NormalisableRange< float > decayRange( 0.1, 10, 0.001);
    decayRange.setSkewForCentre( 3 );
    
    juce::NormalisableRange< float > outputRange( -100, 0, 0.01);
    outputRange.setSkewForCentre( -12 );
    
    static constexpr int pIDVersionNumber = 1;
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "noiseAmount", pIDVersionNumber }, "NoiseAmount", 0, 100, 20 ) );
    
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "noiseLPF", pIDVersionNumber }, "NoiseLPF", 0, 100, 70 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "exciteExtension", pIDVersionNumber }, "ExciteExtension", extensionRange, 1 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "exciteExponent", pIDVersionNumber }, "ExciteExponent", exponentRange, 1 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "exciteCentre", pIDVersionNumber }, "ExciteCentre", 0, 1, 0.1 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "brightness", pIDVersionNumber }, "Brightness", 0, 100, 80 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "decay", pIDVersionNumber }, "Decay", decayRange, 1 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "sensorPos", pIDVersionNumber }, "SensorPos", 0, 100, 20 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "stiffness", pIDVersionNumber }, "Stiffness", -100, 100, 0 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "split", pIDVersionNumber }, "Split", 0, 100, 50 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "harmonic", pIDVersionNumber }, "Harmonic", 0, 100, 0 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "buzz", pIDVersionNumber }, "Buzz", 0, 100, 0 ) );
    params.add( std::make_unique<juce::AudioParameterBool>( juce::ParameterID{ "buzzOnOff", pIDVersionNumber }, "BuzzOnOff", false ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "jitter", pIDVersionNumber }, "Jitter", 0, 100, 0 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "output", pIDVersionNumber }, "Output", outputRange, -6 ) );
    params.add( std::make_unique<juce::AudioParameterFloat>( juce::ParameterID{ "stereoSpread", pIDVersionNumber }, "StereoSpread", 0, 100, 0 ) );
    
    
    return params;
}


void Sjf_fyzikAudioProcessor::setParameters()
{
    m_exciteNoise = *noiseAmountParameter * 0.01;
    m_exciteBright = calculateLPFCoefficient( 75*pow(2, *noiseLPFParameter *0.08 ), getSampleRate() );
    m_exciteEnvC = *exciteCentreParameter;
    m_exciteEnvE = *exciteExponentParameter;
    m_exciteEnvExt = *exciteExtensionParameter;
    m_medBright = *mediumBrightnessParameter * 0.01;
    m_decay = *decayParameter;
    m_split = (*splitParameter * 0.008) + 0.1;
    m_stiff = *stiffnessParameter * 0.009;
    m_sensorPos = (*sensorParameter * 0.008) + 0.1;
    m_nonLinFactor = *buzzParameter * 0.009;
    m_nonLinOn = *buzzOnOffParameter;
    m_harmonic = *harmonicParameter * 0.002;
    m_jitter = *jitterParameter;
    
    if ( m_outputLevel != *outputLevelParameter  )
    {
        m_outputLevel = *outputLevelParameter;
        m_outputSmoother.setTargetValue( pow( 10.0, m_outputLevel/20.0 ) );
    }
    
    m_stereoSpread = *spreadParameter * 0.01;
}
