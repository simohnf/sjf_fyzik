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
#define WIDTH SLIDERSIZE*8 + INDENT*5
#define HEIGHT SLIDERSIZE*2 + TEXT_HEIGHT*4 + INDENT

//==============================================================================
Sjf_fyzikAudioProcessorEditor::Sjf_fyzikAudioProcessorEditor (Sjf_fyzikAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState( vts )
{
    setLookAndFeel( &otherLandF );
    
//    attSlider, decSlider, susSlider, relSlider;
    
    addAndMakeVisible( decaySlider );
    decaySlider.setSliderStyle( juce::Slider::Rotary );
    decaySlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, decaySlider.getWidth(), TEXT_HEIGHT );
    decaySlider.setRange( 0.1, 15 );
    decaySlider.setTextValueSuffix( "s" );
    decaySliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "decay", decaySlider ) );
    decaySlider.setTooltip( "This sets the (approximate) decay time in seconds... other parameters will also impact the decay though...");
    
    addAndMakeVisible( &exciteNoiseLevelSlider );
    exciteNoiseLevelSlider.setSliderStyle( juce::Slider::Rotary );
    exciteNoiseLevelSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, exciteNoiseLevelSlider.getWidth(), TEXT_HEIGHT );
    exciteNoiseLevelSlider.setRange( 0 , 100 );
    exciteNoiseLevelSlider.setTextValueSuffix( "%" );
    exciteNoiseLevelSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "noiseAmount", exciteNoiseLevelSlider ) );
    exciteNoiseLevelSlider.setTooltip( "This sets the percentage of noise in the excitation" );
    
    addAndMakeVisible( &mediumBrightnessSlider );
    mediumBrightnessSlider.setSliderStyle( juce::Slider::Rotary );
    mediumBrightnessSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, mediumBrightnessSlider.getWidth(), TEXT_HEIGHT );
    mediumBrightnessSlider.setTextValueSuffix( "%" );
    mediumBrightnessSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "brightness", mediumBrightnessSlider ) );
    mediumBrightnessSlider.setTooltip( "This sets the brightness of the medium. At higher settings high frequencies will ring longer and vice versa" );
    
    addAndMakeVisible( &splitSlider );
    splitSlider.setSliderStyle( juce::Slider::Rotary );
    splitSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, splitSlider.getWidth(), TEXT_HEIGHT );
    splitSlider.setDoubleClickReturnValue( true, 50 );
    splitSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "split", splitSlider ) );
    splitSlider.setTooltip( "This sets the division of the total period of the travelling wave between the forward and backeards travelling waves. \nNo real physical relationship to this, but it can create some interesting results in combination with the stiffness" );
    
    addAndMakeVisible( &stiffSlider );
    stiffSlider.setSliderStyle( juce::Slider::Rotary );
    stiffSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, mediumBrightnessSlider.getWidth(), TEXT_HEIGHT );
    stiffSlider.setDoubleClickReturnValue( true, 0 );
    stiffSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "stiffness", stiffSlider ) );
    stiffSlider.setTooltip( "This sets the stiffness, and thus inharmonicity, of the medium. Go low or high for more bell/cymbal like sounds" );
    
    addAndMakeVisible( &sensorSlider );
    sensorSlider.setSliderStyle( juce::Slider::Rotary );
    sensorSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, sensorSlider.getWidth(), TEXT_HEIGHT );
    sensorSlider.setTextValueSuffix( "%" );
    sensorSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "sensorPos", sensorSlider ) );
    sensorSlider.setTooltip( "This changes the pickup positon and can create subtle variations to the sound" );
    
    addAndMakeVisible( &envCentreSlider );
    envCentreSlider.setSliderStyle( juce::Slider::LinearBar );
    envCentreSlider.setTextBoxStyle( juce::Slider::NoTextBox, true, 0, 0 );
    envCentreSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "exciteCentre", envCentreSlider ) );
    envCentreSlider.setTooltip( "This changes the centre point of the envelope applied to the excitation" );
    
    addAndMakeVisible( &envPowerSlider );
    envPowerSlider.setSliderStyle( juce::Slider::LinearBar );
    envPowerSlider.setTextBoxStyle( juce::Slider::NoTextBox, true, 0, 0 );
    envPowerSlider.setSkewFactorFromMidPoint( 2 );
    envPowerSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "exciteExponent", envPowerSlider ) );
    envPowerSlider.setTooltip( "This changes the shape of the envelope applied to the excitation" );
    envPowerSlider.onValueChange = [this]
    {
        envDisplay.setEnvelope( envCentreSlider.getValue(), envPowerSlider.getValue() );
    };
    envCentreSlider.onValueChange = [this]
    {
        envDisplay.setEnvelope( envCentreSlider.getValue(), envPowerSlider.getValue() );
    };
    
    
    
    addAndMakeVisible( &envDisplay );
//    envDisplay.drawWaveform( audioProcessor.getEnvelope() );
    envDisplay.setEnvelope( envCentreSlider.getValue(), envPowerSlider.getValue() );
    envDisplay.setTooltip( "This just displays the envelope applied to the excitation" );
    
    addAndMakeVisible( &excitationLengthSlider );
    excitationLengthSlider.setSliderStyle( juce::Slider::Rotary );
    excitationLengthSlider.setSkewFactorFromMidPoint( 5 );
    excitationLengthSlider.setTextValueSuffix("X");
    excitationLengthSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, excitationLengthSlider.getWidth(), TEXT_HEIGHT );
    excitationLengthSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "exciteExtension", excitationLengthSlider ) );
    excitationLengthSlider.setTooltip( "This will extend the length of the excitation... can sound interesting, but can also result in some artefacts :/");
    
    addAndMakeVisible( &jitSlider );
    jitSlider.setSliderStyle( juce::Slider::Rotary );
    jitSlider.setTextValueSuffix("%");
    jitSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, jitSlider.getWidth(), TEXT_HEIGHT );
    jitSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "jitter", jitSlider ) );
    jitSlider.setTooltip( "This will create random variations of the currently set parameters");
    
    addAndMakeVisible( &nonLinSlider );
    nonLinSlider.setSliderStyle( juce::Slider::Rotary );
    nonLinSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, jitSlider.getWidth(), TEXT_HEIGHT );
    nonLinSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "buzz", nonLinSlider ) );
    nonLinSlider.setTooltip( "This sets the intensity of the nonlinear buzzing sound ");
    
    addAndMakeVisible( &nonLinButton1 );
    nonLinButton1Attachment.reset( new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "buzzOnOff", nonLinButton1 ) );
    nonLinButton1.setTooltip( "This turns the nonlinear buzz on/off");
    
//    harmonicSlider
    addAndMakeVisible( &harmonicSlider );
    harmonicSlider.setSliderStyle( juce::Slider::Rotary );
    harmonicSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, harmonicSlider.getWidth(), TEXT_HEIGHT );
    harmonicSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "harmonic", harmonicSlider ) );
    harmonicSlider.setTooltip( "This will create a harmonic, one octave above the pitch... although other parameters will affect the clarity of this" );
    
    addAndMakeVisible( &excitationLPFSlider );
    excitationLPFSlider.setSliderStyle( juce::Slider::Rotary );
    excitationLPFSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, excitationLPFSlider.getWidth(), TEXT_HEIGHT );
    excitationLPFSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "noiseLPF", excitationLPFSlider ) );
    excitationLPFSlider.setTooltip( "This sets the brightness of the noise component of the excitation" );
    
    addAndMakeVisible( &outputSlider );
    outputSlider.setSliderStyle( juce::Slider::Rotary );
    outputSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, false, outputSlider.getWidth(), TEXT_HEIGHT );
//    outputSlider.setRange( 0, 100 );
    outputSlider.setTextValueSuffix( "dB" );
    outputSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "output", outputSlider ) );
    outputSlider.setTooltip( "This sets the output level" );
    
    
    addAndMakeVisible( &stereoSpreadSlider );
    stereoSpreadSlider.setSliderStyle( juce::Slider::LinearBar );
    stereoSpreadSlider.setTextBoxStyle( juce::Slider::NoTextBox, true, 0, 0 );
    stereoSpreadSlider.setTextValueSuffix( "%" );
    stereoSpreadSliderAttachment.reset( new juce::AudioProcessorValueTreeState::SliderAttachment( valueTreeState, "stereoSpread", stereoSpreadSlider ) );
    stereoSpreadSlider.setTooltip( "This sets the stereo spread within which each new note will be randomly place" );
    
    addAndMakeVisible( &tooltipLabel );
    tooltipLabel.setColour( juce::Label::backgroundColourId, otherLandF.backGroundColour.withAlpha( 0.85f ) );
    
    addAndMakeVisible( &tooltipsToggle );
    tooltipsToggle.setTooltip( MAIN_TOOLTIP );
    tooltipsToggle.setButtonText( "HINTS" );
    tooltipsToggle.onClick = [this]
    {
        if (tooltipsToggle.getToggleState())
        {
            tooltipLabel.setVisible( true );
            setSize (WIDTH, HEIGHT+tooltipLabel.getHeight());
        }
        else
        {
            tooltipLabel.setVisible( false );
            setSize (WIDTH, HEIGHT);
        }
    };
    tooltipsToggle.setTooltip( "This turns on/off the hints");
    
    
    setSize ( WIDTH, HEIGHT );
    startTimer( 100 );
}

Sjf_fyzikAudioProcessorEditor::~Sjf_fyzikAudioProcessorEditor()
{
    setLookAndFeel( nullptr );
}

//==============================================================================
void Sjf_fyzikAudioProcessorEditor::paint (juce::Graphics& g)
{
#ifdef JUCE_DEBUG
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
#else
    juce::Rectangle<int> r = { (int)( WIDTH ), (int)(HEIGHT + tooltipLabel.getHeight()) };
    sjf_makeBackground< 40 >( g, r );
#endif
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll (getLookAnd Feel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText( "sjf_fyzik", 0, 0, getWidth(), TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "EXCITATION", exciteNoiseLevelSlider.getX(), exciteNoiseLevelSlider.getY() - TEXT_HEIGHT*2, SLIDERSIZE*3, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Noise Amount", exciteNoiseLevelSlider.getX(), exciteNoiseLevelSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Noise Brightness", excitationLPFSlider.getX(), excitationLPFSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Extension", excitationLengthSlider.getX(), excitationLengthSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "Envelope", envPowerSlider.getX(), envPowerSlider.getY() - TEXT_HEIGHT, SLIDERSIZE*3, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "Exponent", envPowerSlider.getX(), envPowerSlider.getY(), SLIDERSIZE*3, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Centre", envCentreSlider.getX(), envCentreSlider.getY(), SLIDERSIZE*3, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "MEDIUM", mediumBrightnessSlider.getX(), mediumBrightnessSlider.getY() - TEXT_HEIGHT*2, SLIDERSIZE*3, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Brightness", mediumBrightnessSlider.getX(), mediumBrightnessSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Decay", decaySlider.getX(), decaySlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Sensor", sensorSlider.getX(), sensorSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "Stiffness", stiffSlider.getX(), stiffSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Split", splitSlider.getX(), splitSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Harmonic", harmonicSlider.getX(), harmonicSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "Buzz", nonLinSlider.getX(), nonLinSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "Jitter", jitSlider.getX(), jitSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    
    g.drawFittedText( "Output", outputSlider.getX(), outputSlider.getY() - TEXT_HEIGHT, SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    g.drawFittedText( "Spread", stereoSpreadSlider.getX(), stereoSpreadSlider.getY(), SLIDERSIZE, TEXT_HEIGHT, juce::Justification::centred, 1);
    
}

void Sjf_fyzikAudioProcessorEditor::resized()
{
    exciteNoiseLevelSlider.setBounds( INDENT, INDENT*2 + TEXT_HEIGHT*2, SLIDERSIZE, SLIDERSIZE );
    excitationLPFSlider.setBounds( exciteNoiseLevelSlider.getRight(), exciteNoiseLevelSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    excitationLengthSlider.setBounds( excitationLPFSlider.getRight(), excitationLPFSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    
    envPowerSlider.setBounds( exciteNoiseLevelSlider.getX(), exciteNoiseLevelSlider.getBottom() + TEXT_HEIGHT, SLIDERSIZE*3, TEXT_HEIGHT );
    envCentreSlider.setBounds( envPowerSlider.getX(), envPowerSlider.getBottom(), SLIDERSIZE*3, TEXT_HEIGHT );
    envDisplay.setBounds( envCentreSlider.getX(), envCentreSlider.getBottom(), SLIDERSIZE*3, SLIDERSIZE - TEXT_HEIGHT*2 );
    
    mediumBrightnessSlider.setBounds( excitationLengthSlider.getRight() + INDENT, excitationLengthSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    decaySlider.setBounds( mediumBrightnessSlider.getRight(), mediumBrightnessSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    sensorSlider.setBounds( decaySlider.getRight(), decaySlider.getY(), SLIDERSIZE, SLIDERSIZE );
    
    
    stiffSlider.setBounds( mediumBrightnessSlider.getX(), mediumBrightnessSlider.getBottom() + TEXT_HEIGHT, SLIDERSIZE, SLIDERSIZE );
    splitSlider.setBounds( stiffSlider.getRight(), stiffSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    harmonicSlider.setBounds( splitSlider.getRight(), splitSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    
    nonLinSlider.setBounds( sensorSlider.getRight() + INDENT, sensorSlider.getY(), SLIDERSIZE, SLIDERSIZE );
    nonLinButton1.setBounds( nonLinSlider.getRight() - TEXT_HEIGHT, nonLinSlider.getY(), TEXT_HEIGHT, TEXT_HEIGHT );
    
    jitSlider.setBounds( nonLinSlider.getX(), nonLinSlider.getBottom() + TEXT_HEIGHT, SLIDERSIZE, SLIDERSIZE );
    
    outputSlider.setBounds( nonLinSlider.getRight() + INDENT, nonLinSlider.getY() + SLIDERSIZE*0.5 - TEXT_HEIGHT, SLIDERSIZE, SLIDERSIZE );
    stereoSpreadSlider.setBounds( outputSlider.getX(), outputSlider.getBottom(), SLIDERSIZE, TEXT_HEIGHT );
    
    tooltipsToggle.setBounds( stereoSpreadSlider.getX(), stereoSpreadSlider.getBottom() + INDENT, SLIDERSIZE, TEXT_HEIGHT );
    tooltipLabel.setBounds( 0, HEIGHT, getWidth(), TEXT_HEIGHT*4 );
}


void Sjf_fyzikAudioProcessorEditor::timerCallback()
{
    if ( tooltipsToggle.getToggleState() )
        sjf_setTooltipLabel( this, MAIN_TOOLTIP, tooltipLabel );
    
//    envDisplay.setEnvelope( envCentreSlider.getValue(), envPowerSlider.getValue() );
}
