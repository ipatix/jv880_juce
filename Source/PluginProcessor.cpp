/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#include <format>

#include "PluginProcessor.h"
#include "PluginEditor.h"

static std::vector<const uint8_t *> expansions = {
    (const uint8_t*)BinaryData::rd500_expansion_bin,
    (const uint8_t*)BinaryData::jd990_expansion_bin,
    (const uint8_t*)BinaryData::SRJV8001_Pop__CS_0x3F1CF705_bin,
    (const uint8_t*)BinaryData::SRJV8002_Orchestral__CS_0x3F0E09E2_BIN,
    (const uint8_t*)BinaryData::SRJV8003_Piano__CS_0x3F8DB303_bin,
    (const uint8_t*)BinaryData::SRJV8004_Vintage_Synth__CS_0x3E23B90C_BIN,
    (const uint8_t*)BinaryData::SRJV8005_World__CS_0x3E8E8A0D_bin,
    (const uint8_t*)BinaryData::SRJV8006_Dance__CS_0x3EC462E0_bin,
    (const uint8_t*)BinaryData::SRJV8007_Super_Sound_Set__CS_0x3F1EE208_bin,
    (const uint8_t*)BinaryData::SRJV8008_Keyboards_of_the_60s_and_70s__CS_0x3F1E3F0A_BIN,
    (const uint8_t*)BinaryData::SRJV8009_Session__CS_0x3F381791_BIN,
    (const uint8_t*)BinaryData::SRJV8010_Bass__Drum__CS_0x3D83D02A_BIN,
    (const uint8_t*)BinaryData::SRJV8011_Techno__CS_0x3F046250_bin,
    (const uint8_t*)BinaryData::SRJV8012_HipHop__CS_0x3EA08A19_BIN,
    (const uint8_t*)BinaryData::SRJV8013_Vocal__CS_0x3ECE78AA_bin,
    (const uint8_t*)BinaryData::SRJV8014_Asia__CS_0x3C8A1582_bin,
    (const uint8_t*)BinaryData::SRJV8015_Special_FX__CS_0x3F591CE4_bin,
    (const uint8_t*)BinaryData::SRJV8016_Orchestral_II__CS_0x3F35B03B_bin,
    (const uint8_t*)BinaryData::SRJV8017_Country__CS_0x3ED75089_bin,
    (const uint8_t*)BinaryData::SRJV8018_Latin__CS_0x3EA51033_BIN,
    (const uint8_t*)BinaryData::SRJV8019_House__CS_0x3E330C41_BIN
};

static void unscramble(const uint8_t *src, uint8_t *dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        int address = i & ~0xfffff;
        static const int aa[] = {
            2, 0, 3, 4, 1, 9, 13, 10, 18, 17, 6, 15, 11, 16, 8, 5, 12, 7, 14, 19
        };
        for (int j = 0; j < 20; j++)
        {
            if (i & (1 << j))
                address |= 1<<aa[j];
        }
        uint8_t srcdata = src[address];
        uint8_t data = 0;
        static const int dd[] = {
            2, 0, 4, 5, 7, 6, 3, 1
        };
        for (int j = 0; j < 8; j++)
        {
            if (srcdata & (1 << dd[j]))
                data |= 1<<j;
        }
        dst[i] = data;
    }
}

//==============================================================================
Jv880_juceAudioProcessor::Jv880_juceAudioProcessor()
     : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      )
{
    mcu = new MCU();
    mcu->startSC55(BinaryData::jv880_rom1_bin, BinaryData::jv880_rom2_bin,
                   BinaryData::jv880_waverom1_bin, BinaryData::jv880_waverom2_bin,
                   BinaryData::jv880_nvram_bin);

    //std::vector<std::pair<size_t, const char *>> descrambleList = {
    //    { 2, "SR-JV80-01 Pop - CS 0x3F1CF705.bin" },
    //    { 3, "SR-JV80-02 Orchestral - CS 0x3F0E09E2.BIN" },
    //    { 4, "SR-JV80-03 Piano - CS 0x3F8DB303.bin" },
    //    { 5, "SR-JV80-04 Vintage Synth - CS 0x3E23B90C.BIN" },
    //    { 6, "SR-JV80-05 World - CS 0x3E8E8A0D.bin" },
    //    { 7, "SR-JV80-06 Dance - CS 0x3EC462E0.bin" },
    //    { 8, "SR-JV80-07 Super Sound Set - CS 0x3F1EE208.bin" },
    //    { 9, "SR-JV80-08 Keyboards of the 60s and 70s - CS 0x3F1E3F0A.BIN" },
    //    { 10, "SR-JV80-09 Session - CS 0x3F381791.BIN" },
    //    { 11, "SR-JV80-10 Bass & Drum - CS 0x3D83D02A.BIN" },
    //    { 12, "SR-JV80-11 Techno - CS 0x3F046250.bin" },
    //    { 13, "SR-JV80-12 HipHop - CS 0x3EA08A19.BIN" },
    //    { 14, "SR-JV80-13 Vocal - CS 0x3ECE78AA.bin" },
    //    { 15, "SR-JV80-14 Asia - CS 0x3C8A1582.bin" },
    //    { 16, "SR-JV80-15 Special FX - CS 0x3F591CE4.bin" },
    //    { 17, "SR-JV80-16 Orchestral II - CS 0x3F35B03B.bin" },
    //    { 18, "SR-JV80-17 Country - CS 0x3ED75089.bin" },
    //    { 19, "SR-JV80-18 Latin - CS 0x3EA51033.BIN" },
    //    { 20, "SR-JV80-19 House - CS 0x3E330C41.BIN" },
    //    //{ 21, "SR-JV80-90 Cus99 CS_0x404FAD63.bin" },
    //    //{ 22, "SR-JV80-97 Experience III - CS 0x0FE3E621.BIN" },
    //    //{ 23, "SR-JV80-98_Experience_II 0x0FBBEA47.BIN" },
    //    //{ 24, "SR-JV80-99_Experience 0x0FC21498.BIN" },
    //};

    //for (size_t i = 0; i < descrambleList.size(); i++) {
    //    const uint8_t *src = expansions.at(descrambleList.at(i).first);
    //    const char *name = descrambleList.at(i).second;
    //    std::vector<uint8_t> dest(0x800000);
    //    unscramble(src, dest.data(), dest.size());
    //    std::string outpath = std::format("/tmp/{}", name);
    //    FILE *f = fopen(outpath.c_str(), "wb");
    //    fwrite(dest.data(), dest.size(), 1, f);
    //    fclose(f);
    //}

    // Internal User
    patchInfoPerGroup.emplace_back();
    for (int j = 0; j < 64; j++)
    {
        patchInfos.emplace_back(std::make_unique<PatchInfo>());
        patchInfos.back()->ptr = &BinaryData::jv880_rom2_bin[0x008ce0 + j * 0x16a];
        patchInfos.back()->name = std::string(patchInfos.back()->ptr, 12);
        patchInfos.back()->expansionI = 0xff;
        patchInfos.back()->patchI = j;
        patchInfos.back()->present = true;
        patchInfos.back()->drums = false;
        patchInfos.back()->iInList = patchInfos.size() - 1;
        patchInfoPerGroup.back().emplace_back(patchInfos.back());
    }
    patchInfos.emplace_back(std::make_unique<PatchInfo>());
    patchInfos.back()->name = "Drums Internal User"; // why did nameLength used to be 21 and not 19?, check other Drum names as well when changing
    patchInfos.back()->ptr = &BinaryData::jv880_rom2_bin[0x00e760];
    patchInfos.back()->expansionI = 0xff;
    patchInfos.back()->patchI = 0;
    patchInfos.back()->present = true;
    patchInfos.back()->drums = true;
    patchInfos.back()->iInList = patchInfos.size() - 1;
    patchInfoPerGroup.back().emplace_back(patchInfos.back());
    
    // Internal A
    for (int j = 0; j < 64; j++)
    {
        patchInfos.emplace_back(std::make_unique<PatchInfo>());
        patchInfos.back()->ptr = &BinaryData::jv880_rom2_bin[0x010ce0 + j * 0x16a];
        patchInfos.back()->name = std::string(patchInfos.back()->ptr, 12);
        patchInfos.back()->expansionI = 0xff;
        patchInfos.back()->patchI = j;
        patchInfos.back()->present = true;
        patchInfos.back()->drums = false;
        patchInfos.back()->iInList = patchInfos.size() - 1;
        patchInfoPerGroup.back().emplace_back(patchInfos.back());
    }
    patchInfos.emplace_back(std::make_unique<PatchInfo>());
    patchInfos.back()->name = "Drums Internal A";
    patchInfos.back()->ptr = &BinaryData::jv880_rom2_bin[0x016760];
    patchInfos.back()->expansionI = 0xff;
    patchInfos.back()->patchI = 0;
    patchInfos.back()->present = true;
    patchInfos.back()->drums = true;
    patchInfos.back()->iInList = patchInfos.size() - 1;
    patchInfoPerGroup.back().emplace_back(patchInfos.back());
    
    // Internal B
    for (int j = 0; j < 64; j++)
    {
        patchInfos.emplace_back(std::make_unique<PatchInfo>());
        patchInfos.back()->ptr = &BinaryData::jv880_rom2_bin[0x018ce0 + j * 0x16a];
        patchInfos.back()->name = std::string(patchInfos.back()->ptr, 12);
        patchInfos.back()->expansionI = 0xff;
        patchInfos.back()->patchI = j;
        patchInfos.back()->present = true;
        patchInfos.back()->drums = false;
        patchInfos.back()->iInList = patchInfos.size() - 1;
        patchInfoPerGroup.back().emplace_back(patchInfos.back());
    }
    patchInfos.emplace_back(std::make_unique<PatchInfo>());
    patchInfos.back()->name = "Drums Internal B";
    patchInfos.back()->ptr = &BinaryData::jv880_rom2_bin[0x01e760];
    patchInfos.back()->expansionI = 0xff;
    patchInfos.back()->patchI = 0.12;
    patchInfos.back()->present = true;
    patchInfos.back()->drums = true;
    patchInfos.back()->iInList = patchInfos.size() - 1;
    patchInfoPerGroup.back().emplace_back(patchInfos.back());

    for (size_t i = 0; i < expansions.size(); i++)
    {
        patchInfoPerGroup.emplace_back();

        const uint8_t *desc = expansionsDescr.emplace_back(expansions.at(i));

        //printf("[%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x]\n", desc[0], desc[1], desc[2], desc[3], desc[4], desc[5], desc[6], desc[7]);

        // get patches
        int nPatches = desc[0x67] | desc[0x66] << 8;
        if (i == 0) nPatches = 192; // RD-500
        //printf("exp=%d nPatches=%d\n", i, nPatches);
        for (int j = 0; j < nPatches; j++)
        {
            size_t patchesOffset = desc[0x8f] | desc[0x8e] << 8
                                 | desc[0x8d] << 16 | desc[0x8c] << 24;
            // RD-500
            if (i == 0 && j < 64) patchesOffset = 0x0ce0;
            else if (i == 0 && j < 128) patchesOffset = 0x8370;
            else if (i == 0) patchesOffset = 0x12b82;
            patchInfos.emplace_back(std::make_unique<PatchInfo>());
            if (i == 0)
                patchInfos.back()->ptr = reinterpret_cast<const char *>(&BinaryData::rd500_patches_bin[patchesOffset + (j % 64) * 0x16a]);
            else
                patchInfos.back()->ptr = reinterpret_cast<const char *>(&desc[patchesOffset + j * 0x16a]);
            bool err = false;
            for (size_t ci = 0; ci < 12; ci++) {
                //printf("ptr=%p\n", patchInfos.back()->ptr);
                char c = patchInfos.back()->ptr[ci];
                if (c == 0)
                    break;
                if (!strchr("abcdefghijklmnopqrstuvwqxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -+./", c)) {
                    //printf("Expansion %d patch %d contains invalid char: '%c' (%02x)\n", i, j, c, (uint8_t)c);
                    err = true;
                    break;
                }
            }
            if (err)
                patchInfos.back()->name = std::format("ERROR EXP={} PATCH={}", i, j);
            else
                patchInfos.back()->name = std::string(patchInfos.back()->ptr, 12);
            patchInfos.back()->expansionI = i;
            patchInfos.back()->patchI = j;
            patchInfos.back()->present = true;
            patchInfos.back()->drums = false;
            patchInfos.back()->iInList = patchInfos.size() - 1;
            patchInfoPerGroup.back().emplace_back(patchInfos.back());
        }

        // get drumkits
        int nDrumkits = desc[0x69] | desc[0x68] << 8;
        //printf("exp=%d nDrumkits=%d\n", i, nDrumkits);
        if (i == 0) nDrumkits = 3; // RD-500
        for (int j = 0; j < nDrumkits; j++)
        {
            size_t patchesOffset = desc[0x93] | desc[0x92] << 8
                | desc[0x91] << 16 | desc[0x90] << 24;
            // RD-500
            if (i == 0 && j < 64) patchesOffset = 0x6760;
            else if (i == 0 && j < 128) patchesOffset = 0xd2a0;
            else if (i == 0) patchesOffset = 0x18602;
            patchInfos.emplace_back(std::make_unique<PatchInfo>());
            patchInfos.back()->name = std::format("Exp {} Drums {}", i, j);
            if (i == 0)
                patchInfos.back()->ptr = reinterpret_cast<const char *>(&BinaryData::rd500_patches_bin[patchesOffset]);
            else
                patchInfos.back()->ptr = reinterpret_cast<const char *>(&desc[patchesOffset + j * 0xa7c]);
            patchInfos.back()->expansionI = i;
            patchInfos.back()->patchI = j;
            patchInfos.back()->present = true;
            patchInfos.back()->drums = true;
            patchInfos.back()->iInList = patchInfos.size() - 1;
            patchInfoPerGroup.back().emplace_back(patchInfos.back());
        }

        // total count
        totalPatchesExp += nPatches;
    }
}

Jv880_juceAudioProcessor::~Jv880_juceAudioProcessor()
{
    memset(mcu, 0, sizeof(MCU));
    delete mcu;
}

//==============================================================================
const juce::String Jv880_juceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Jv880_juceAudioProcessor::acceptsMidi() const
{
    return true;
}

bool Jv880_juceAudioProcessor::producesMidi() const
{
    return false;
}

bool Jv880_juceAudioProcessor::isMidiEffect() const
{
    return false;
}

double Jv880_juceAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Jv880_juceAudioProcessor::getNumPrograms()
{
    return patchInfos.size();
}

int Jv880_juceAudioProcessor::getCurrentProgram()
{
    return 0; // TODO
}

void Jv880_juceAudioProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    int expansionI = patchInfos[index]->expansionI;
    if (expansionI != 0xff && status.currentExpansion != expansionI)
    {
        status.currentExpansion = expansionI;
        memcpy(mcu->pcm.waverom_exp, expansionsDescr[expansionI], 0x800000);
        mcu->SC55_Reset();
    }

    if (patchInfos[index]->drums)
    {
        status.isDrums = true;
        mcu->nvram[0x11] = 0;
        memcpy(&mcu->nvram[0x67f0], (uint8_t*)patchInfos[index]->ptr, 0xa7c);
        memcpy(status.drums, &mcu->nvram[0x67f0], 0xa7c);
        mcu->SC55_Reset();
    }
    else
    {
        status.isDrums = false;
        if (mcu->nvram[0x11] != 1)
        {
            mcu->nvram[0x11] = 1;
            memcpy(&mcu->nvram[0x0d70], (uint8_t*)patchInfos[index]->ptr, 0x16a);
            memcpy(status.patch, &mcu->nvram[0x0d70], 0x16a);
            mcu->SC55_Reset();
        }
        else
        {
            memcpy(&mcu->nvram[0x0d70], (uint8_t*)patchInfos[index]->ptr, 0x16a);
            memcpy(status.patch, &mcu->nvram[0x0d70], 0x16a);
            uint8_t buffer[2] = { 0xC0, 0x00 };
            mcu->postMidiSC55(buffer, sizeof(buffer));
        }
    }
}

const juce::String Jv880_juceAudioProcessor::getProgramName (int index)
{
    return patchInfos[index]->name;
}

void Jv880_juceAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Jv880_juceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Jv880_juceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool Jv880_juceAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void Jv880_juceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (status.isDrums)
            message.setChannel(10);
        else
            message.setChannel(1);
        int samplePos = (double)metadata.samplePosition / getSampleRate() * 64000;
        mcu->enqueueMidiSC55(message.getRawData(), message.getRawDataSize(), samplePos);
    }
 
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);
    mcu->updateSC55WithSampleRate(channelDataL, channelDataR, buffer.getNumSamples(), getSampleRate());
}

//==============================================================================
bool Jv880_juceAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Jv880_juceAudioProcessor::createEditor()
{
    return new Jv880_juceAudioProcessorEditor (*this);
}

//==============================================================================
void Jv880_juceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    status.masterTune = mcu->nvram[0x00];
    status.reverbEnabled = ((mcu->nvram[0x02] >> 0) & 1) == 1;
    status.chorusEnabled = ((mcu->nvram[0x02] >> 1) & 1) == 1;
    
    destData.ensureSize(sizeof(DataToSave));
    destData.replaceAll(&status, sizeof(DataToSave));
}

void Jv880_juceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    memcpy(&status, data, sizeof(DataToSave));

    mcu->nvram[0x0d] |= 1 << 5; // LastSet
    mcu->nvram[0x00] = status.masterTune;
    mcu->nvram[0x02] = status.reverbEnabled | status.chorusEnabled << 1;

    memcpy(mcu->pcm.waverom_exp, expansionsDescr[status.currentExpansion], 0x800000);
    mcu->nvram[0x11] = status.isDrums ? 0 : 1;
    memcpy(&mcu->nvram[0x67f0], status.drums, 0xa7c);
    memcpy(&mcu->nvram[0x0d70], status.patch, 0x16a);
}

void Jv880_juceAudioProcessor::sendSysexParamChange(uint32_t address, uint8_t value)
{
    uint8_t data[5];
    data[0] = (address >> 21) & 127; // address MSB
    data[1] = (address >> 14) & 127; // address
    data[2] = (address >> 7) & 127;  // address
    data[3] = (address >> 0) & 127;  // address LSB
    data[4] = value;  // data
    uint32_t checksum = 0;
    for (size_t i = 0; i < 5; i++) {
        checksum += data[i];
        if (checksum >= 128) {
            checksum -= 128;
        }
    }

    uint8_t buf[12];
    buf[0] = 0xf0;
    buf[1] = 0x41;
    buf[2] = 0x10; // unit number
    buf[3] = 0x46;
    buf[4] = 0x12; // command
    checksum = 128 - checksum;
    for (size_t i = 0; i < 5; i++) {
        buf[i + 5] = data[i];
    }
    buf[10] = checksum;
    buf[11] = 0xf7;

    mcu->postMidiSC55(buf, 12);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Jv880_juceAudioProcessor();
}
