#include "PluginProcessor.h"
#include "PluginEditor.h"

AECompressorProcessor::AECompressorProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AECompressorProcessor::createLayout()
{
    using FloatParameter = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"input", 1}, "Input",
                                                juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f, "dB"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"threshold", 1}, "Threshold",
                                                juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -18.0f, "dB"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"ratio", 1}, "Ratio",
                                                juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.45f), 4.0f));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"attack", 1}, "Attack",
                                                juce::NormalisableRange<float>(0.1f, 200.0f, 0.1f, 0.35f), 20.0f, "ms"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"release", 1}, "Release",
                                                juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.35f), 120.0f, "ms"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"makeup", 1}, "Make-up",
                                                juce::NormalisableRange<float>(-12.0f, 18.0f, 0.1f), 0.0f, "dB"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"output", 1}, "Output",
                                                juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f, "dB"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"mix", 1}, "Mix",
                                                juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f, "%"));
    layout.add(std::make_unique<FloatParameter>(juce::ParameterID{"bassIgnore", 1}, "Bass Ignore",
                                                juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.45f), 120.0f, "Hz"));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"knee", 1}, "Knee",
                                                            juce::StringArray{"Soft", "Medium", "Hard"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"autoGain", 1}, "Auto Gain", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass", 1}, "Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"preset", 1}, "Preset",
                                                            juce::StringArray{"Vocal", "Kick", "Bass", "Snare", "Drum Bus", "Mix Bus", "Master", "Parallel"}, 0));
    return layout;
}

void AECompressorProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    dsp.prepare(sampleRate, maximumExpectedSamplesPerBlock, getTotalNumOutputChannels());
}

bool AECompressorProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    return (input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo())
           && input == layouts.getMainOutputChannelSet();
}

void AECompressorProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (apvts.getRawParameterValue("bypass")->load() > 0.5f)
        return;

    CompressorDSP::Params params;
    params.inputDb = apvts.getRawParameterValue("input")->load();
    params.thresholdDb = apvts.getRawParameterValue("threshold")->load();
    params.ratio = apvts.getRawParameterValue("ratio")->load();
    params.attackMs = apvts.getRawParameterValue("attack")->load();
    params.releaseMs = apvts.getRawParameterValue("release")->load();
    params.makeupDb = apvts.getRawParameterValue("makeup")->load();
    params.mix = apvts.getRawParameterValue("mix")->load();
    params.bassIgnoreHz = apvts.getRawParameterValue("bassIgnore")->load();
    params.knee = static_cast<int>(apvts.getRawParameterValue("knee")->load());
    params.autoGain = apvts.getRawParameterValue("autoGain")->load() > 0.5f;

    dsp.setParams(params);
    dsp.process(buffer);
    dsp.applyOutputGain(buffer, apvts.getRawParameterValue("output")->load());
}

void AECompressorProcessor::applyPreset(int index)
{
    struct Preset
    {
        float threshold, ratio, attack, release, makeup, mix, bassIgnore;
        int knee;
        bool autoGain;
    };

    static constexpr Preset presets[] = {
        {-18.0f, 4.0f, 20.0f, 120.0f, 0.0f, 100.0f, 120.0f, 0, true},
        {-12.0f, 5.0f, 25.0f, 90.0f, 0.0f, 100.0f, 45.0f, 2, false},
        {-16.0f, 4.0f, 30.0f, 150.0f, 0.0f, 100.0f, 35.0f, 1, false},
        {-14.0f, 6.0f, 12.0f, 100.0f, 0.0f, 100.0f, 90.0f, 1, false},
        {-10.0f, 2.5f, 30.0f, 180.0f, 0.0f, 100.0f, 100.0f, 0, false},
        {-8.0f, 2.0f, 35.0f, 220.0f, 0.0f, 100.0f, 120.0f, 0, false},
        {-6.0f, 1.5f, 50.0f, 300.0f, 0.0f, 100.0f, 150.0f, 0, false},
        {-24.0f, 8.0f, 5.0f, 100.0f, 0.0f, 35.0f, 100.0f, 1, true}
    };

    index = juce::jlimit(0, 7, index);
    const auto& preset = presets[index];

    const auto set = [this](const char* id, float value)
    {
        if (auto* parameter = apvts.getParameter(id))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };

    set("threshold", preset.threshold);
    set("ratio", preset.ratio);
    set("attack", preset.attack);
    set("release", preset.release);
    set("makeup", preset.makeup);
    set("mix", preset.mix);
    set("bassIgnore", preset.bassIgnore);
    set("knee", static_cast<float>(preset.knee));
    set("autoGain", preset.autoGain ? 1.0f : 0.0f);
    set("preset", static_cast<float>(index));
}

void AECompressorProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void AECompressorProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* AECompressorProcessor::createEditor()
{
    return new AECompressorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AECompressorProcessor();
}
