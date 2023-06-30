/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define INDENT 10
#define SLIDERSIZE 100
#define TEXT_HEIGHT 20
//==============================================================================
Sjf_karplusStrongAudioProcessorEditor::Sjf_karplusStrongAudioProcessorEditor (Sjf_karplusStrongAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel( &otherLandF );
    
//    attSlider, decSlider, susSlider, relSlider;
    
    addAndMakeVisible( attSlider );
    attSlider.setSliderStyle( juce::Slider::Rotary );
    attSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, attSlider.getWidth(), TEXT_HEIGHT );
    attSlider.setRange( 0, 500 );
    attSlider.onValueChange = [ this ]
    {
//        audioProcessor.setEnvelope( Sjf_karplusStrongAudioProcessor::envelopeIndices::attack, attSlider.getValue() );
        audioProcessor.setAttackTime( attSlider.getValue() );
    };
  
    
    addAndMakeVisible( &attackBrightnessSlider );
    attackBrightnessSlider.setSliderStyle( juce::Slider::Rotary );
    attackBrightnessSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, attackBrightnessSlider.getWidth(), TEXT_HEIGHT );
    attackBrightnessSlider.setRange( 0 , 100 );
    attackBrightnessSlider.setTextValueSuffix( "%" );
    attackBrightnessSlider.onValueChange = [ this ]
    {
        audioProcessor.setAttackBrightness( attackBrightnessSlider.getValue() );
    };
    
    addAndMakeVisible( &mediumBrightnessSlider );
    mediumBrightnessSlider.setSliderStyle( juce::Slider::Rotary );
    mediumBrightnessSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, mediumBrightnessSlider.getWidth(), TEXT_HEIGHT );
    mediumBrightnessSlider.setRange( 0 , 100 );
    mediumBrightnessSlider.setTextValueSuffix( "%" );
    mediumBrightnessSlider.onValueChange = [ this ]
    {
        audioProcessor.setMediumBrightness( mediumBrightnessSlider.getValue() );
    };
    
    addAndMakeVisible( &blendSlider );
    blendSlider.setSliderStyle( juce::Slider::Rotary );
    blendSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, mediumBrightnessSlider.getWidth(), TEXT_HEIGHT );
    blendSlider.setRange( 0, 100 );
    blendSlider.onValueChange = [ this ]
    {
        audioProcessor.setBlend( blendSlider.getValue() );
    };
    
    
    addAndMakeVisible( &driveSlider );
    driveSlider.setSliderStyle( juce::Slider::Rotary );
    driveSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, mediumBrightnessSlider.getWidth(), TEXT_HEIGHT );
    driveSlider.setRange( 0, 100 );
    driveSlider.onValueChange = [ this ]
    {
        audioProcessor.setDrive( driveSlider.getValue() );
    };
    
    
    addAndMakeVisible( &extendHighsButton );
    extendHighsButton.setButtonText( "highs" );
    extendHighsButton.onClick = [ this ]
    {
        audioProcessor.extendHighs( extendHighsButton.getToggleState() );
    };
    
    addAndMakeVisible( &retriggerButton );
    retriggerButton.setButtonText( "retrigger" );
//    retriggerButton.onClick = [ this ]
//    {
//        audioProcessor.retriggerNotes( retriggerButton.getToggleState() );
//    };
    
    addAndMakeVisible( &randomTableButton );
    randomTableButton.setButtonText( "rand" );
    randomTableButton.onClick = [ this ]
    {
        audioProcessor.randomiseWavetable( randomTableButton.getToggleState() );
    };
    
    addAndMakeVisible( &harpButton );
    harpButton.setButtonText( "harp" );
    harpButton.onClick = [ this ]
    {
        audioProcessor.shouldTriggerNoteOffs( harpButton.getToggleState() );
    };
    setSize (400, 300);
}

Sjf_karplusStrongAudioProcessorEditor::~Sjf_karplusStrongAudioProcessorEditor()
{
    setLookAndFeel( nullptr );
}

//==============================================================================
void Sjf_karplusStrongAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Sjf_karplusStrongAudioProcessorEditor::resized()
{
    attackBrightnessSlider.setBounds( INDENT, TEXT_HEIGHT, SLIDERSIZE, SLIDERSIZE );
    mediumBrightnessSlider.setBounds( attackBrightnessSlider.getRight(), attackBrightnessSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    blendSlider.setBounds( mediumBrightnessSlider.getRight(), mediumBrightnessSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    
    extendHighsButton.setBounds( blendSlider.getRight(), blendSlider.getY(), SLIDERSIZE, TEXT_HEIGHT );
    retriggerButton.setBounds( extendHighsButton.getX(), extendHighsButton.getBottom(), SLIDERSIZE, TEXT_HEIGHT );
    randomTableButton.setBounds( retriggerButton.getX(), retriggerButton.getBottom(), SLIDERSIZE, TEXT_HEIGHT );
    harpButton.setBounds( randomTableButton.getX(), randomTableButton.getBottom(), SLIDERSIZE, TEXT_HEIGHT );
    
    
    attSlider.setBounds( attackBrightnessSlider.getX(), attackBrightnessSlider.getBottom() + INDENT + TEXT_HEIGHT, SLIDERSIZE, SLIDERSIZE );
    driveSlider.setBounds( attSlider.getRight(), attSlider.getY(), SLIDERSIZE, SLIDERSIZE );
}
