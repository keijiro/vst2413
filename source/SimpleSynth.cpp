#include "SimpleSynth.h"
#include <cstdio>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new SimpleSynth(audioMaster);
}

SimpleSynth::SimpleSynth(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, kNumPrograms, kNumParameters),
    sampleRate_(44100.0f),
    velocity_(0.0f),
    phase_(0.0f),
    noteCount_(0),
    note_(0) {
    if(audioMaster != NULL) {
        setNumInputs(kNumInputs);
        setNumOutputs(kNumOutputs);
        setUniqueID(kUniqueId);
        canProcessReplacing();
        isSynth();
    }
    suspend();
}

SimpleSynth::~SimpleSynth() {
}

VstInt32 SimpleSynth::canDo(char *text) {
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

bool SimpleSynth::copyProgram(long destination) {
  // TODO: Copy program to destination
  return false;
}

VstInt32 SimpleSynth::getCurrentMidiProgram(VstInt32 channel, MidiProgramName *mpn) {
	VstInt32 prg = 0;
  // TODO: Look up your current MIDI program and fill the MidiProgramName with it
	return prg;
}

bool SimpleSynth::getEffectName(char* name) {
  strncpy(name, "SimpleSynth", kVstMaxProductStrLen);
  return true;
}

bool SimpleSynth::getMidiKeyName(VstInt32 channel, MidiKeyName *key) {
	// TODO: Fill in this information
  // key->thisProgramIndex;		// >= 0. fill struct for this program index.
	// key->thisKeyNumber;			// 0 - 127. fill struct for this key number.
	key->keyName[0] = 0;
	key->reserved = 0;				// zero
	key->flags = 0;					// reserved, none defined yet, zero.
	return true;
}

VstInt32 SimpleSynth::getMidiProgramCategory(VstInt32 channel, MidiProgramCategory *category) {
  // TODO: Get the MIDI program category
	return 0;
}

VstInt32 SimpleSynth::getMidiProgramName(VstInt32 channel, MidiProgramName *mpn) {
  // TODO: Return the MIDI program name
  return 0;
}

VstInt32 SimpleSynth::getNumMidiInputChannels() {
  // TODO: Change this value for polyphonic synths
  return 1;
}

VstInt32 SimpleSynth::getNumMidiOutputChannels() {
  // TODO: Change this value if you are sending MIDI output back to the host
  return 0;
}

bool SimpleSynth::getOutputProperties(VstInt32 index, VstPinProperties *properties) {
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

float SimpleSynth::getParameter(VstInt32 index) {
  // TODO: Get parameter value for index
  return 0.0;
}

void SimpleSynth::getParameterDisplay(VstInt32 index, char *text) {
  // TODO: Get parameter display for index
}

void SimpleSynth::getParameterLabel(VstInt32 index, char *text) {
  // TODO: Get parameter label for index
}

void SimpleSynth::getParameterName(VstInt32 index, char *text) {
  // TODO: Get parameter name for index
}

VstPlugCategory SimpleSynth::getPlugCategory() { 
  return kPlugCategSynth;
}

bool SimpleSynth::getProductString(char* text) {
  // TODO: Replace with actual description of your synth
  strncpy(text, "SimpleSynth", kVstMaxProductStrLen);
  return true;
}

void SimpleSynth::getProgramName(char *name) {
  // TODO: Copy active program name into "name" string
}

bool SimpleSynth::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text) {
  // TODO: Return program name for index
  return false;
}

bool SimpleSynth::getVendorString(char* text) {
  strncpy(text, "RadiumSoftware", kVstMaxVendorStrLen);
  return true;
}

VstInt32 SimpleSynth::getVendorVersion() {
  // TODO: Return actual version of this plugin
  return 1000;
}

bool SimpleSynth::hasMidiProgramsChanged(VstInt32 channel) {
  // TODO: Return true/false if the MIDI programs have changed
  return false;
}

void SimpleSynth::setBlockSize(VstInt32 blockSize) {
  // TODO: Handle this call if necessary
	AudioEffectX::setBlockSize(blockSize);
}

void SimpleSynth::setParameter(VstInt32 index, float value) {
  // TODO: Set parameter value for index
}

void SimpleSynth::setProgram(VstInt32 index) {
  // TOOD: Set local variables based on program parameters
}

void SimpleSynth::setProgramName(char *name) {
  // TODO: Set program name
}

void SimpleSynth::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    sampleRate_ = sampleRate;
}
