#include "vst2413.h"

#pragma mark Creation and destruction

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Vst2413(audioMaster);
}

Vst2413::Vst2413(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, Driver::kProgramMax, Driver::kParamMax),
    driver_(44100)
{
    if(audioMaster != NULL) {
        setNumInputs(0);
        setNumOutputs(1);
        setUniqueID(kUniqueId);
        canProcessReplacing();
        isSynth();
    }
    suspend();
}

Vst2413::~Vst2413() {
}

#pragma mark
#pragma mark Processing functions

VstInt32 Vst2413::processEvents(VstEvents* events) {
	for (VstInt32 i = 0; i < events->numEvents; i++) {
		if (events->events[i]->type != kVstMidiType) continue;

		const char* data = reinterpret_cast<VstMidiEvent*>(events->events[i])->midiData;
        
        switch (data[0] & 0xf0) {
            // key off
            case 0x80:
                driver_.KeyOff(data[1] & 0x7f);
                break;
            // key On
            case 0x90:
                driver_.KeyOn(data[1] & 0x7f, 1.0f / 128 * (data[2] & 0x7f));
                break;
            // all keys off
            case 0xb0:
                if (data[1] == 0x7e || data[1] == 0x7b) driver_.KeyOffAll();
                break;
            // pitch wheel
            case 0xe0: {
                int position = ((data[2] & 0x7f) << 7) + (data[1] & 0x7f);
                driver_.SetPitchWheel((1.0f / 0x2000) * (position - 0x2000));
                break;
            }
            default:
                break;
        }
	}
	return 1;
}

void Vst2413::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
    for (VstInt32 i = 0; i < sampleFrames; i++) outputs[0][i] = driver_.Step();
}

#pragma mark
#pragma mark Program

void Vst2413::setProgram(VstInt32 index) {
    driver_.SetProgram(static_cast<Driver::ProgramID>(index));
}

void Vst2413::setProgramName(char* name) {
    // not supported
}

void Vst2413::getProgramName(char* name) {
    driver_.GetProgramName(driver_.GetProgram()).copy(name, kVstMaxProgNameLen);
}

bool Vst2413::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text) {
    if (index < Driver::kProgramMax) {
        driver_.GetProgramName(static_cast<Driver::ProgramID>(index)).copy(text, kVstMaxProgNameLen);
        return true;
    } else {
        return false;
    }
}

#pragma mark
#pragma mark Parameter

void Vst2413::setParameter(VstInt32 index, float value) {
    driver_.SetParameter(static_cast<Driver::ParameterID>(index), value);
}

float Vst2413::getParameter(VstInt32 index) {
    return driver_.GetParameter(static_cast<Driver::ParameterID>(index));
}

void Vst2413::getParameterLabel(VstInt32 index, char* text) {
    driver_.GetParameterLabel(static_cast<Driver::ParameterID>(index)).copy(text, kVstMaxParamStrLen);
}

void Vst2413::getParameterDisplay(VstInt32 index, char* text) {
    driver_.GetParameterText(static_cast<Driver::ParameterID>(index)).copy(text, kVstMaxParamStrLen);
}

void Vst2413::getParameterName(VstInt32 index, char* text) {
    driver_.GetParameterName(static_cast<Driver::ParameterID>(index)).copy(text, kVstMaxParamStrLen);
}

#pragma mark
#pragma mark Output settings

void Vst2413::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    driver_.SetSampleRate(sampleRate);
}

void Vst2413::setBlockSize(VstInt32 blockSize) {
    AudioEffectX::setBlockSize(blockSize);
}

bool Vst2413::getOutputProperties(VstInt32 index, VstPinProperties* properties) {
    if (index == 0) {
        Driver::String("1 Out").copy(properties->label, kVstMaxLabelLen);
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

#pragma mark
#pragma mark Plug-in properties

bool Vst2413::getEffectName(char* name) {
    Driver::String("VST2413").copy(name, kVstMaxEffectNameLen);
    return true;
}

bool Vst2413::getVendorString(char* text) {
    Driver::String("RadiumSoftware").copy(text, kVstMaxVendorStrLen);
    return true;
}

bool Vst2413::getProductString(char* text) {
    Driver::String("VST2413").copy(text, kVstMaxProductStrLen);
    return true;
}

VstInt32 Vst2413::getVendorVersion() {
    return 1000;
}

VstInt32 Vst2413::canDo(char* text) {
    Driver::String str = text;
    if (str == "receiveVstEvents") return 1;
    if (str == "receiveVstMidiEvent") return 1;
    if (str == "midiProgramNames") return 1;
    return 0;
}

#pragma mark
#pragma mark MIDI channels I/O

VstInt32 Vst2413::getNumMidiInputChannels() {
    return 9;
}

VstInt32 Vst2413::getNumMidiOutputChannels() {
    return 0;
}

#pragma mark
#pragma mark MIDI program

VstInt32 Vst2413::getMidiProgramName(VstInt32 channel, MidiProgramName* mpn) {
    return 0;
}

VstInt32 Vst2413::getCurrentMidiProgram(VstInt32 channel, MidiProgramName* mpn) {
    return 0;
}

VstInt32 Vst2413::getMidiProgramCategory(VstInt32 channel, MidiProgramCategory* category) {
    return 0;
}

bool Vst2413::hasMidiProgramsChanged(VstInt32 channel) {
    return false;
}

bool Vst2413::getMidiKeyName(VstInt32 channel, MidiKeyName* key) {
	key->keyName[0] = 0;
	key->reserved = 0;
	key->flags = 0;
	return false;
}
