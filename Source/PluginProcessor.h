/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../sjf_audio/sjf_waveguide.h"
#include "../sjf_audio/sjf_lpf.h"

#define NVOICES 16
//==============================================================================
/**
*/
class Sjf_fyzikAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Sjf_fyzikAudioProcessor();
    ~Sjf_fyzikAudioProcessor() override;

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


    void shouldTriggerNoteOffs( bool trueIfTriggersNoteOffs )
    {
        m_triggerNoteOffs = trueIfTriggersNoteOffs;
    }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    
    void setParameters();
    
    std::array< sjf_waveguide, NVOICES > m_strings;
//    std::array< std::array< double, 2 >, NVOICES > m_pan;
    size_t m_voiceNum = 0;
    std::array< sjf_lpf< double >, 2 > m_dcBlock;
    bool m_triggerNoteOffs = false;
    
    
    double m_exciteNoise = 0, m_exciteBright = 0, m_exciteEnvC = 0, m_exciteEnvE = 0, m_exciteEnvExt = 0, m_medBright = 0, m_decay = 0, m_split = 0, m_stiff = 0, m_sensorPos = 0, m_nonLinFactor = 0, m_harmonic = 0, m_jitter = 0, m_outputLevel = -6, m_stereoSpread = 0;
    bool m_nonLinOn = false;
    
    std::array< std::array< juce::SmoothedValue< float >, 2 >, NVOICES > m_panSmoothers;
    juce::SmoothedValue< float > m_outputSmoother;
    
    std::atomic<float>* noiseAmountParameter = nullptr;
    std::atomic<float>* noiseLPFParameter = nullptr;
    std::atomic<float>* exciteExtensionParameter = nullptr;
    std::atomic<float>* exciteExponentParameter = nullptr;
    std::atomic<float>* exciteCentreParameter = nullptr;
    std::atomic<float>* mediumBrightnessParameter = nullptr;
    std::atomic<float>* decayParameter = nullptr;
    std::atomic<float>* sensorParameter = nullptr;
    std::atomic<float>* stiffnessParameter = nullptr;
    std::atomic<float>* splitParameter = nullptr;
    std::atomic<float>* harmonicParameter = nullptr;
    std::atomic<float>* buzzParameter = nullptr;
    std::atomic<float>* buzzOnOffParameter = nullptr;
    std::atomic<float>* jitterParameter = nullptr;
    std::atomic<float>* outputLevelParameter = nullptr;
    std::atomic<float>* spreadParameter = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_fyzikAudioProcessor)
};
