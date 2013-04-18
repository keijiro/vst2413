#include "Vst2413p.h"

namespace {
    typedef std::string String;
    
    // Converts a float value to a SynthDriver program ID.
    SynthDriver::ProgramID ValueToProgramID(float value) {
        int range = SynthDriver::kPrograms - SynthDriver::kProgramFirstPreset - 1;
        return static_cast<SynthDriver::ProgramID>(static_cast<int>(value * range + SynthDriver::kProgramFirstPreset));
    }

    // Converts a VST parameter index to a SynthDriver parameter ID.
    SynthDriver::ParameterID IndexToParameterID(int parameterIndex) {
        return (parameterIndex == 1) ? SynthDriver::kParameterWheelRange : SynthDriver::kParameterFineTune;
    }
}

#pragma mark Creation and destruction

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Vst2413p(audioMaster);
}

Vst2413p::Vst2413p(audioMasterCallback audioMaster)
:   AudioEffectX(audioMaster, 0, 3), // only 3 parameters are supported
    driver_(44100),
    instrumentParameter_(0)
{
    if(audioMaster != NULL) {
        setNumInputs(0);
        setNumOutputs(1);
        setUniqueID(kUniqueId);
        canProcessReplacing();
        isSynth();
    }
    driver_.SetProgram(ValueToProgramID(instrumentParameter_));
    suspend();
}

#pragma mark
#pragma mark Processing functions

VstInt32 Vst2413p::processEvents(VstEvents* events) {
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

void Vst2413p::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
    for (VstInt32 i = 0; i < sampleFrames; i++) outputs[0][i] = driver_.Step();
}

#pragma mark
#pragma mark Parameter

void Vst2413p::setParameter(VstInt32 index, float value) {
    if (index == 0) {
        instrumentParameter_ = value;
        driver_.SetProgram(ValueToProgramID(value));
    } else {
        driver_.SetParameter(IndexToParameterID(index), value);
    }
}

float Vst2413p::getParameter(VstInt32 index) {
    if (index == 0) {
        return instrumentParameter_;
    } else {
        return driver_.GetParameter(IndexToParameterID(index));
    }
}

void Vst2413p::getParameterLabel(VstInt32 index, char* text) {
    if (index != 0) {
        vst_strncpy(text, driver_.GetParameterLabel(IndexToParameterID(index)).c_str(), kVstMaxParamStrLen);
    }
}

void Vst2413p::getParameterDisplay(VstInt32 index, char* text) {
    if (index == 0) {
        vst_strncpy(text, driver_.GetProgramName(ValueToProgramID(instrumentParameter_)).c_str(), kVstMaxParamStrLen);
    } else {
        vst_strncpy(text, driver_.GetParameterText(IndexToParameterID(index)).c_str(), kVstMaxParamStrLen);
    }
}

void Vst2413p::getParameterName(VstInt32 index, char* text) {
    if (index == 0) {
        vst_strncpy(text, "Instrument", kVstMaxParamStrLen);
    } else {
        vst_strncpy(text, driver_.GetParameterName(IndexToParameterID(index)).c_str(), kVstMaxParamStrLen);
    }
}

#pragma mark
#pragma mark Output settings

void Vst2413p::setSampleRate(float sampleRate) {
	AudioEffectX::setSampleRate(sampleRate);
    driver_.SetSampleRate(sampleRate);
}

bool Vst2413p::getOutputProperties(VstInt32 index, VstPinProperties* properties) {
    if (index == 0) {
        vst_strncpy(properties->label, "1 Out", kVstMaxLabelLen);
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

#pragma mark
#pragma mark Plug-in properties

bool Vst2413p::getEffectName(char* name) {
    vst_strncpy(name, "VST2413P", kVstMaxEffectNameLen);
    return true;
}

bool Vst2413p::getVendorString(char* text) {
    vst_strncpy(text, "Radium Software", kVstMaxVendorStrLen);
    return true;
}

bool Vst2413p::getProductString(char* text) {
    vst_strncpy(text, "VST2413P", kVstMaxProductStrLen);
    return true;
}

VstInt32 Vst2413p::getVendorVersion() {
    return 1000;
}

VstInt32 Vst2413p::canDo(char* text) {
    String str = text;
    if (str == "receiveVstEvents") return 1;
    if (str == "receiveVstMidiEvent") return 1;
    return 0;
}

#pragma mark
#pragma mark MIDI channels I/O

VstInt32 Vst2413p::getNumMidiInputChannels() {
    return 1;
}

VstInt32 Vst2413p::getNumMidiOutputChannels() {
    return 0;
}
