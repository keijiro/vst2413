#include "vst2413.h"
#include <cstdio>

namespace {
    const int kNumPrograms = 16;
    const unsigned long kUniqueId = 'dAzy';
}

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Vst2413(audioMaster);
}

Vst2413::Vst2413(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, kNumPrograms, Driver::kParamMax),
    driver_(44100),
    program_(0)
{
    if(audioMaster != NULL) {
        setNumInputs(0);
        setNumOutputs(1);
        setUniqueID(kUniqueId);
        canProcessReplacing();
        isSynth();
    }
    suspend();
    driver_.SetProgram(program_);
}

Vst2413::~Vst2413() {
}

VstInt32 Vst2413::canDo(char *text) {
    if(!strcmp(text, "receiveVstEvents")) return 1;
    if(!strcmp(text, "receiveVstMidiEvent")) return 1;
    if(!strcmp(text, "midiProgramNames")) return 1;
    return 0;
}

bool Vst2413::copyProgram(long destination) {
  // TODO: Copy program to destination
  return false;
}

VstInt32 Vst2413::getCurrentMidiProgram(VstInt32 channel, MidiProgramName *mpn) {
    return 0;
}

bool Vst2413::getEffectName(char* name) {
    strncpy(name, "VST2413", kVstMaxProductStrLen);
    return true;
}

bool Vst2413::getMidiKeyName(VstInt32 channel, MidiKeyName *key) {
	key->keyName[0] = 0;
	key->reserved = 0;
	key->flags = 0;
	return false;
}

VstInt32 Vst2413::getMidiProgramCategory(VstInt32 channel, MidiProgramCategory *category) {
    return 0;
}

VstInt32 Vst2413::getMidiProgramName(VstInt32 channel, MidiProgramName *mpn) {
    return 0;
}

VstInt32 Vst2413::getNumMidiInputChannels() {
    return 9;
}

VstInt32 Vst2413::getNumMidiOutputChannels() {
    return 0;
}

bool Vst2413::getOutputProperties(VstInt32 index, VstPinProperties *properties) {
    if (index == 0) {
        strncpy(properties->label, "1 Out", kVstMaxLabelLen);
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

float Vst2413::getParameter(VstInt32 index) {
    return driver_.GetParameter(index);
}

void Vst2413::getParameterDisplay(VstInt32 index, char *text) {
    strncpy(text, driver_.GetParameterText(index).c_str(), kVstMaxParamStrLen);
}

void Vst2413::getParameterLabel(VstInt32 index, char *text) {
    strncpy(text, driver_.GetParameterLabel(index), kVstMaxParamStrLen);
}

void Vst2413::getParameterName(VstInt32 index, char *text) {
    strncpy(text, driver_.GetParameterName(index), kVstMaxParamStrLen);
}

void Vst2413::setParameter(VstInt32 index, float value) {
    driver_.SetParameter(index, value);
}

VstPlugCategory Vst2413::getPlugCategory() {
    return kPlugCategSynth;
}

bool Vst2413::getProductString(char* text) {
    strncpy(text, "VST2413", kVstMaxProductStrLen);
    return true;
}

void Vst2413::getProgramName(char *name) {
    strncpy(name, driver_.GetProgramName(program_), kVstMaxProductStrLen);
}

bool Vst2413::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text) {
    strncpy(text, driver_.GetProgramName(index), kVstMaxProductStrLen);
    return false;
}

bool Vst2413::getVendorString(char* text) {
    strncpy(text, "RadiumSoftware", kVstMaxVendorStrLen);
    return true;
}

VstInt32 Vst2413::getVendorVersion() {
    return 1000;
}

bool Vst2413::hasMidiProgramsChanged(VstInt32 channel) {
    return false;
}

void Vst2413::setBlockSize(VstInt32 blockSize) {
    AudioEffectX::setBlockSize(blockSize);
}

void Vst2413::setProgram(VstInt32 index) {
    program_ = index;
    driver_.SetProgram(index);
}

void Vst2413::setProgramName(char *name) {
    // TODO: Set program name
}

void Vst2413::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    driver_.SetSampleRate(sampleRate);
}
