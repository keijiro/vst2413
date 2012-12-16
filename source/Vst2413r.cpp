#include "Vst2413r.h"

namespace {
    typedef std::string String;
}

#pragma mark Creation and destruction

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Vst2413r(audioMaster);
}

Vst2413r::Vst2413r(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, 0, 0),
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

#pragma mark
#pragma mark Processing functions

VstInt32 Vst2413r::processEvents(VstEvents* events) {
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
            default:
                break;
        }
	}
	return 1;
}

void Vst2413r::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
    for (VstInt32 i = 0; i < sampleFrames; i++) outputs[0][i] = driver_.Step();
}

#pragma mark
#pragma mark Output settings

void Vst2413r::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    driver_.SetSampleRate(sampleRate);
}

bool Vst2413r::getOutputProperties(VstInt32 index, VstPinProperties* properties) {
    if (index == 0) {
        String("1 Out").copy(properties->label, kVstMaxLabelLen);
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

#pragma mark
#pragma mark Plug-in properties

bool Vst2413r::getEffectName(char* name) {
    String("VST2413R").copy(name, kVstMaxEffectNameLen);
    return true;
}

bool Vst2413r::getVendorString(char* text) {
    String("Radium Software").copy(text, kVstMaxVendorStrLen);
    return true;
}

bool Vst2413r::getProductString(char* text) {
    String("VST2413R").copy(text, kVstMaxProductStrLen);
    return true;
}

VstInt32 Vst2413r::getVendorVersion() {
    return 1000;
}

VstInt32 Vst2413r::canDo(char* text) {
    String str = text;
    if (str == "receiveVstEvents") return 1;
    if (str == "receiveVstMidiEvent") return 1;
    return 0;
}

#pragma mark
#pragma mark MIDI channels I/O

VstInt32 Vst2413r::getNumMidiInputChannels() {
    return 1;
}

VstInt32 Vst2413r::getNumMidiOutputChannels() {
    return 0;
}
