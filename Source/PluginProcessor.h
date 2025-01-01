/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <JuceHeader.h>
#include "emulator/mcu.h"

//==============================================================================
/**
*/
class Jv880_juceAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Jv880_juceAudioProcessor();
    ~Jv880_juceAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

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

    //==============================================================================
    void sendSysexParamChange(uint32_t address, uint8_t value);

    struct PatchInfo
    {
        std::string name;
        const char *ptr = nullptr;
        int expansionI = 0; // 0xff: no expansion
        int patchI = 0;
        bool present = false;
        bool drums = false;
        size_t iInList = 0;
    };

    struct DataToSave
    {
        int8_t masterTune = 0;
        bool reverbEnabled = 1;
        bool chorusEnabled = 1;

        int currentExpansion = 0;
        bool isDrums = false;
        uint8_t patch[0x16a] = {0};
        uint8_t drums[0xa7c] = {0};
    };

    DataToSave status;
    MCU *mcu;
    std::vector<const char *> expansionsDescr;
    std::vector<std::shared_ptr<PatchInfo>> patchInfos;
    std::vector<std::vector<std::shared_ptr<PatchInfo>>> patchInfoPerGroup;
    int totalPatchesExp = 0;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Jv880_juceAudioProcessor)
};
