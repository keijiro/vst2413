#include "vst2413.h"
#include <cstdio>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Vst2413(audioMaster);
}

Vst2413::Vst2413(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, kNumPrograms, kNumParameters),
    driver_(44100)
{
    if(audioMaster != NULL) {
        setNumInputs(kNumInputs);
        setNumOutputs(kNumOutputs);
        setUniqueID(kUniqueId);
        canProcessReplacing();
        isSynth();
    }
    suspend();
}

Vst2413::~Vst2413() {
}

VstInt32 Vst2413::canDo(char *text) {
  // TODO: Fill in according to your plugin's capabilities
  if(!strcmp(text, "receiveVstEvents")) {
		return 1;
  }
  else if(!strcmp(text, "receiveVstMidiEvent")) {
		return 1;
  }
	else if(!strcmp(text, "midiProgramNames")) {
		return 1;
  }
  
  // -1 => explicitly can't do; 0 => don't know
	return 0;
}

bool Vst2413::copyProgram(long destination) {
  // TODO: Copy program to destination
  return false;
}

VstInt32 Vst2413::getCurrentMidiProgram(VstInt32 channel, MidiProgramName *mpn) {
	VstInt32 prg = 0;
  // TODO: Look up your current MIDI program and fill the MidiProgramName with it
	return prg;
}

bool Vst2413::getEffectName(char* name) {
  strncpy(name, "Vst2413", kVstMaxProductStrLen);
  return true;
}

bool Vst2413::getMidiKeyName(VstInt32 channel, MidiKeyName *key) {
	// TODO: Fill in this information
  // key->thisProgramIndex;		// >= 0. fill struct for this program index.
	// key->thisKeyNumber;			// 0 - 127. fill struct for this key number.
	key->keyName[0] = 0;
	key->reserved = 0;				// zero
	key->flags = 0;					// reserved, none defined yet, zero.
	return true;
}

VstInt32 Vst2413::getMidiProgramCategory(VstInt32 channel, MidiProgramCategory *category) {
  // TODO: Get the MIDI program category
	return 0;
}

VstInt32 Vst2413::getMidiProgramName(VstInt32 channel, MidiProgramName *mpn) {
  // TODO: Return the MIDI program name
  return 0;
}

VstInt32 Vst2413::getNumMidiInputChannels() {
  // TODO: Change this value for polyphonic synths
  return 1;
}

VstInt32 Vst2413::getNumMidiOutputChannels() {
  // TODO: Change this value if you are sending MIDI output back to the host
  return 0;
}

bool Vst2413::getOutputProperties(VstInt32 index, VstPinProperties *properties) {
  if(index < kNumOutputs) {
    sprintf(properties->label, "%1d Out", index + 1);
		properties->flags = kVstPinIsActive;
		if(index < 2) {
			properties->flags |= kVstPinIsStereo;
    }
    return true;
  }
  return false;
}

float Vst2413::getParameter(VstInt32 index) {
  // TODO: Get parameter value for index
  return 0.0;
}

void Vst2413::getParameterDisplay(VstInt32 index, char *text) {
  // TODO: Get parameter display for index
}

void Vst2413::getParameterLabel(VstInt32 index, char *text) {
  // TODO: Get parameter label for index
}

void Vst2413::getParameterName(VstInt32 index, char *text) {
  // TODO: Get parameter name for index
}

VstPlugCategory Vst2413::getPlugCategory() { 
  return kPlugCategSynth;
}

bool Vst2413::getProductString(char* text) {
  // TODO: Replace with actual description of your synth
  strncpy(text, "Vst2413", kVstMaxProductStrLen);
  return true;
}

void Vst2413::getProgramName(char *name) {
  // TODO: Copy active program name into "name" string
}

bool Vst2413::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text) {
  // TODO: Return program name for index
  return false;
}

bool Vst2413::getVendorString(char* text) {
  strncpy(text, "RadiumSoftware", kVstMaxVendorStrLen);
  return true;
}

VstInt32 Vst2413::getVendorVersion() {
  // TODO: Return actual version of this plugin
  return 1000;
}

bool Vst2413::hasMidiProgramsChanged(VstInt32 channel) {
  // TODO: Return true/false if the MIDI programs have changed
  return false;
}

void Vst2413::setBlockSize(VstInt32 blockSize) {
  // TODO: Handle this call if necessary
	AudioEffectX::setBlockSize(blockSize);
}

void Vst2413::setParameter(VstInt32 index, float value) {
  // TODO: Set parameter value for index
}

void Vst2413::setProgram(VstInt32 index) {
  // TOOD: Set local variables based on program parameters
}

void Vst2413::setProgramName(char *name) {
  // TODO: Set program name
}

void Vst2413::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    driver_.SetSampleRate(sampleRate);
}
