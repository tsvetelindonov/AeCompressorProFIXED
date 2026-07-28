#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CompressorDSP.h"

class AECompressorProcessor : public juce::AudioProcessor
{
public:
    AECompressorProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override{return true;}
    const juce::String getName() const override{return "AE Compressor";}
    bool acceptsMidi() const override{return false;}
    bool producesMidi() const override{return false;}
    double getTailLengthSeconds() const override{return 0.0;}
    int getNumPrograms() override{return 1;}
    int getCurrentProgram() override{return 0;}
    void setCurrentProgram(int) override{}
    const juce::String getProgramName(int) override{return {};}
    void changeProgramName(int,const juce::String&) override{}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    void applyPreset(int);
    float reductionDb()const{return dsp.getReductionDb();}
    float inputDb()const{return dsp.getInputPeakDb();}
    float outputDb()const{return dsp.getOutputPeakDb();}
    juce::AudioProcessorValueTreeState apvts;
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    CompressorDSP dsp;
};
