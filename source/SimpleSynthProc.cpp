#include "SimpleSynth.h"
#include <math.h>

namespace {
    const float kPi = 3.14159265359f;
}

VstInt32 SimpleSynth::processEvents(VstEvents* events) {
	for (VstInt32 i = 0; i < events->numEvents; i++) {
		if ((events->events[i])->type != kVstMidiType) continue;
        
		const VstMidiEvent* ev = reinterpret_cast<VstMidiEvent*>(events->events[i]);
		const char* midiData = ev->midiData;
		VstInt32 status = midiData[0] & 0xf0;
        
        if (status == 0x90) {
            note_ = midiData[1] & 0x7f;
            velocity_ = midiData[2] & 0x7f;
            noteCount_++;
        } else if (status == 0x80) {
            if (noteCount_ > 0) noteCount_--;
        } else if (status == 0xb0) {
            if (midiData[1] == 0x7e || midiData[1] == 0x7b) {
                noteCount_ = 0;
            }
        }
	}
	return 1;
}

void SimpleSynth::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    float* out = outputs[0];
    
    if (noteCount_ > 0) {
        const float freq = 440.0f * powf(2.0f, (1.0f / 12) * (note_ - 69));
        const float delta = freq / sampleRate_;
        while (--sampleFrames >= 0) {
            (*out++) = sin(kPi * 2 * phase_);
            phase_ += delta;
            if (phase_ > 1.0f) phase_ -= 1.0f;
        }
    } else {
        while (--sampleFrames >= 0) (*out++) = 0.0f;
    }
}
