#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
class AECompressorEditor:public juce::AudioProcessorEditor,private juce::Timer
{
public: explicit AECompressorEditor(AECompressorProcessor&); ~AECompressorEditor()override{setLookAndFeel(nullptr);} void paint(juce::Graphics&)override; void resized()override;
private:
    struct LAF:juce::LookAndFeel_V4{LAF();void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;};
    void timerCallback()override; void knob(juce::Slider&,juce::Label&,const juce::String&,const juce::String&); void meter(juce::Graphics&,juce::Rectangle<float>,float,bool,const juce::String&);
    AECompressorProcessor& p;LAF laf;juce::Slider input,threshold,output,mix,ratio,attack,release,makeup,bass;juce::Label il,tl,ol,ml,rl,al,rel,makl,bl;juce::TextButton soft{"SOFT"},medium{"MEDIUM"},hard{"HARD"};juce::ToggleButton autoGain{"AUTO GAIN"},bypass{"BYPASS"};std::array<juce::TextButton,8> presets;float gr=0,in= -80,out=-80;
    using SA=juce::AudioProcessorValueTreeState::SliderAttachment;using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;std::unique_ptr<SA> a1,a2,a3,a4,a5,a6,a7,a8,a9;std::unique_ptr<BA> b1,b2;
};
