/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../sjf_audio/sjf_LookAndFeel.h"

//==============================================================================
/**
*/
class Sjf_karplusStrongAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Sjf_karplusStrongAudioProcessorEditor (Sjf_karplusStrongAudioProcessor&);
    ~Sjf_karplusStrongAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sjf_karplusStrongAudioProcessor& audioProcessor;
    sjf_lookAndFeel otherLandF;
    
    juce::Slider attackBrightnessSlider, mediumBrightnessSlider, attSlider, blendSlider, driveSlider;
    
    juce::ToggleButton extendHighsButton, retriggerButton, randomTableButton, harpButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_karplusStrongAudioProcessorEditor)
};
