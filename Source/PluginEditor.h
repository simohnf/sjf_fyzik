/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../sjf_audio/sjf_LookAndFeel.h"
//#include "../sjf_audio/sjf_waveform.h"

//==============================================================================
/**
*/
class sjf_envDisplay : public juce::Component, public juce::SettableTooltipClient
{
    //==============================================================================
    //==============================================================================
    //==============================================================================
public:
    sjf_envDisplay()
    {
        
        setInterceptsMouseClicks(false, false);
        setSize(600, 300);
        m_vect.resize( getWidth() );
        for ( auto& i : m_vect )
            i = 0;
    }
    //==============================================================================
    ~sjf_envDisplay(){}
    //==============================================================================
    void drawWaveform( std::vector< double > vectorToDraw )
    {
        if ( vectorToDraw.size() == 0 )
            return;
        for ( auto& i : m_vect )
            i = 0;
        if ( vectorToDraw.size() > m_vect.size() )
        {
            auto scale = static_cast<float>(m_vect.size()) / static_cast<float>(vectorToDraw.size());
            for ( auto i = 0; i < vectorToDraw.size(); i++ )
            {
                auto envIndx = i* scale;
                m_vect[ envIndx ] = vectorToDraw[ i ];
            }
        }
        else
        {
            auto scale =  static_cast<float>(vectorToDraw.size()) / static_cast<float>(m_vect.size());
            for ( auto i = 0; i < m_vect.size(); i++ )
            {
                auto envIndx = i* scale;
                m_vect[ i ] = vectorToDraw[ envIndx ];
            }
        }
        repaint();
    }
    
    //==============================================================================
    void paint ( juce::Graphics& g ) override
    {
        g.setColour ( juce::Colours::grey );
        for ( auto i = 1; i < m_vect.size()-1; i++ )
        {
            auto preIndx = i-1;
            g.drawLine( preIndx, getHeight() - int(m_vect[ preIndx ]*getHeight()), i, getHeight() - int(m_vect[ i ]*getHeight()) );
            g.drawLine( i, getHeight(), i, getHeight() - int(m_vect[ i ]*getHeight() ) );
        }
        
        g.drawRect( 0, 0, getWidth(), getHeight() );
    }
    
    void resized() override
    {
        auto temp = m_vect;
        m_vect.resize( getWidth() );
        for ( auto& i : m_vect )
            i = 0;
        drawWaveform( temp );
    }
    
    void setEnvelope( const double centrePoint, const double exponent )
    {
        if ( m_centre == centrePoint && m_exponent == exponent )
            return;
        m_centre = centrePoint;
        m_exponent = exponent;
        auto nSteps = m_vect.size();
        // first sample in envelope and last samples in envelope are always zero so it always returns to zero
        for ( auto i = 0; i < nSteps; i++)
        {
            auto normalisedPos = static_cast< double >( i ) / static_cast< double >( nSteps ) ;
            m_vect[ i+1 ] =  ( normalisedPos < m_centre ) ? ( normalisedPos / m_centre ) : ( 1 - normalisedPos ) / ( 1 - m_centre );
            m_vect[ i+1 ] = std::pow( m_vect[ i+1 ], exponent );
        }
        drawWaveform( m_vect );
    }
private:
    std::vector< double > m_vect;
    double m_centre = 0, m_exponent = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( sjf_envDisplay )
};

class Sjf_fyzikAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    Sjf_fyzikAudioProcessorEditor (Sjf_fyzikAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Sjf_fyzikAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sjf_fyzikAudioProcessor& audioProcessor;
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    sjf_lookAndFeel otherLandF;
    
    juce::Slider exciteNoiseLevelSlider, mediumBrightnessSlider, decaySlider, splitSlider, stiffSlider, sensorSlider, envCentreSlider, envPowerSlider, excitationLengthSlider, jitSlider, nonLinSlider, harmonicSlider, excitationLPFSlider, outputSlider, stereoSpreadSlider;
    
    juce::ToggleButton nonLinButton1, nonLinButton2, oddHarmsButton;

    sjf_envDisplay envDisplay;
    
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> exciteNoiseLevelSliderAttachment, mediumBrightnessSliderAttachment, decaySliderAttachment, splitSliderAttachment, stiffSliderAttachment, sensorSliderAttachment, envCentreSliderAttachment, envPowerSliderAttachment, excitationLengthSliderAttachment, jitSliderAttachment, nonLinSliderAttachment, harmonicSliderAttachment, excitationLPFSliderAttachment, outputSliderAttachment, stereoSpreadSliderAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> nonLinButton1Attachment;
    
    juce::ToggleButton tooltipsToggle;
    juce::Label tooltipLabel;
    juce::String MAIN_TOOLTIP = "sjf_fyzik: \nPhysical modelling synthesiser \n";
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_fyzikAudioProcessorEditor)
};
