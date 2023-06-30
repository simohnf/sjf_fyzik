/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../sjf_audio/sjf_karplusStrong.h"
#include "../sjf_audio/sjf_lpf.h"
//==============================================================================
/**
*/
class Sjf_karplusStrongAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Sjf_karplusStrongAudioProcessor();
    ~Sjf_karplusStrongAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    
    void randomiseWavetable( bool shouldRandomise )
    {
        for ( auto i = 0; i < m_strings.size(); i++ )
            m_strings[ i ].shouldRandomiseWavetable( shouldRandomise );
    }
    
//    void retriggerNotes( bool retrigger )
//    {
//        for ( auto& string : m_strings )
//            string.shouldRetrigger( retrigger );
//    }
    
    void extendHighs( bool extendHighF )
    {
        for ( auto& string : m_strings )
            string.shouldExtendHighFrequencies( extendHighF );
    }
    
    void setAttackBrightness( double bright )
    {
        for ( auto& string : m_strings )
            string.setAttackBrightness( bright );
    }
    
    void setMediumBrightness( double bright )
    {
        for ( auto& string : m_strings )
            string.setMediumBrightness( bright );
    }
    
    void shouldTriggerNoteOffs( bool trueIfTriggersNoteOffs )
    {
        m_triggerNoteOffs = trueIfTriggersNoteOffs;
    }

    void setAttackTime( double attack )
    {
        auto att = attack * 0.01 * getSampleRate() * 0.1;
        for ( auto& string : m_strings )
            string.setAttackTime( att );
    }
    
    void setBlend( double blend )
    {
        for ( auto& str : m_strings )
            str.setBlend( blend );
    }
    
    void setDrive( double drive )
    {
        for ( auto& str : m_strings )
            str.setDrive( drive );
    }
private:
    //==============================================================================
    
    std::array< sjf_karplusStrongVoice, 32 > m_strings;
    std::array< double, 4 > m_env{ 0, 100, 0, 0 };
    size_t m_voiceNum = 0;
    std::array< sjf_lpf< double >, 2 > m_dcBlock;
    bool m_triggerNoteOffs = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_karplusStrongAudioProcessor)
};
