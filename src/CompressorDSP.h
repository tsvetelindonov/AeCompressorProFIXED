#pragma once
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>
#include <vector>

class CompressorDSP
{
public:
    struct Params
    {
        float inputDb=0, thresholdDb=-18, ratio=4, attackMs=20, releaseMs=120;
        float makeupDb=0, mix=100, bassIgnoreHz=120;
        int knee=0;
        bool autoGain=false;
    };

    void prepare(double sampleRate, int maxBlock, int channels)
    {
        fs=sampleRate;
        dry.setSize(channels, maxBlock);
        hp.assign((size_t)channels, {});
        reset();
    }

    void reset()
    {
        env=0; reduction.store(0); inPeak.store(-80); outPeak.store(-80);
        for(auto& s:hp){s.x1=0;s.y1=0;}
    }

    void setParams(const Params& np){p=np; update();}

    void process(juce::AudioBuffer<float>& b)
    {
        const int n=b.getNumSamples(), chs=b.getNumChannels();
        dry.setSize(chs,n,false,false,true);
        for(int ch=0;ch<chs;++ch) dry.copyFrom(ch,0,b,ch,0,n);

        const float inG=juce::Decibels::decibelsToGain(p.inputDb);
        const float wet=juce::jlimit(0.0f,1.0f,p.mix/100.0f);
        const float makeup=juce::Decibels::decibelsToGain(p.makeupDb+(p.autoGain?autoMakeup():0.0f));
        float bpIn=0,bpOut=0,maxGR=0;

        for(int i=0;i<n;++i)
        {
            float detect=0;
            for(int ch=0;ch<chs;++ch)
            {
                float x=b.getSample(ch,i)*inG;
                b.setSample(ch,i,x);
                bpIn=std::max(bpIn,std::abs(x));
                auto& s=hp[(size_t)ch];
                float y=a0*x+a1*s.x1-b1*s.y1;
                s.x1=x; s.y1=y;
                detect=std::max(detect,std::abs(y));
            }
            const float c=detect>env?att:rel;
            env=detect+c*(env-detect);
            const float levelDb=juce::Decibels::gainToDecibels(std::max(env,1.0e-9f),-120.0f);
            const float gr=gainReduction(levelDb);
            maxGR=std::max(maxGR,gr);
            const float cg=juce::Decibels::decibelsToGain(-gr);
            for(int ch=0;ch<chs;++ch)
            {
                const float w=b.getSample(ch,i)*cg*makeup;
                const float d=dry.getSample(ch,i);
                const float o=d*(1.0f-wet)+w*wet;
                b.setSample(ch,i,o);
                bpOut=std::max(bpOut,std::abs(o));
            }
        }
        reduction.store(maxGR);
        inPeak.store(juce::Decibels::gainToDecibels(bpIn,-80.0f));
        outPeak.store(juce::Decibels::gainToDecibels(bpOut,-80.0f));
    }

    float getReductionDb()const{return reduction.load();}
    float getInputPeakDb()const{return inPeak.load();}
    float getOutputPeakDb()const{return outPeak.load();}

    void applyOutputGain(juce::AudioBuffer<float>& buffer, float outputDb)
    {
        const float gain = juce::Decibels::decibelsToGain(outputDb);
        buffer.applyGain(gain);

        float peak = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            peak = std::max(peak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));

        outPeak.store(juce::Decibels::gainToDecibels(peak, -80.0f));
    }

private:
    struct State{float x1=0,y1=0;};
    void update()
    {
        att=std::exp(-1.0f/((float)fs*std::max(0.0001f,p.attackMs*0.001f)));
        rel=std::exp(-1.0f/((float)fs*std::max(0.001f,p.releaseMs*0.001f)));
        const float fc=juce::jlimit(20.0f,500.0f,p.bassIgnoreHz);
        const float k=std::tan(juce::MathConstants<float>::pi*fc/(float)fs);
        const float q=1.0f/(1.0f+k);
        a0=q; a1=-q; b1=(1.0f-k)*q;
    }
    float kneeWidth()const{return p.knee==0?12.0f:(p.knee==1?6.0f:0.0f);}
    float gainReduction(float levelDb)const
    {
        const float r=std::max(1.0f,p.ratio), over=levelDb-p.thresholdDb, slope=1.0f-1.0f/r, kw=kneeWidth();
        if(kw<=0.001f) return std::max(0.0f,over*slope);
        if(over<=-kw*0.5f) return 0;
        if(over>= kw*0.5f) return over*slope;
        const float x=over+kw*0.5f;
        return slope*x*x/(2.0f*kw);
    }
    float autoMakeup()const
    {
        return std::min(8.0f,std::max(0.0f,-p.thresholdDb)*0.15f*(1.0f-1.0f/std::max(1.0f,p.ratio)));
    }

    Params p; double fs=48000; float env=0,att=0.99f,rel=0.999f,a0=1,a1=-1,b1=0;
    std::vector<State> hp; juce::AudioBuffer<float> dry;
    std::atomic<float> reduction{0},inPeak{-80},outPeak{-80};
};
