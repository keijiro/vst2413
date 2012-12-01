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
            
            static const int fnums[] = { 181, 192, 204, 216, 229, 242, 257, 272, 288, 305, 323, 343 };
            int fnum = fnums[(note_ - 37) % 12];
            int block = (note_ - 37) / 12;
            block = block < 0 ? 0 : (block > 7) ? 7 : block;
            OPLL_writeReg(opll_, 0x10, fnum & 0xff);
            OPLL_writeReg(opll_, 0x20, 0x30 + (fnum >> 8) + (block << 1));
            
            noteCount_++;
        } else if (status == 0x80) {
            if (noteCount_ > 0) noteCount_--;
            
            if (noteCount_ == 0) {
                OPLL_writeReg(opll_, 0x20, 0);
            }
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
    while (--sampleFrames >= 0) (*out++) = (4.0f / 32767) * OPLL_calc(opll_);
}
