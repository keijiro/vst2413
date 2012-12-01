#ifndef __SimpleSynthProc_H
#include "SimpleSynth.h"
#endif

#include <math.h>

static float phase = 0.0f;

VstInt32 SimpleSynth::processEvents (VstEvents* ev) {
  // TODO: Add your MIDI handling code here
	return 1;
}

void SimpleSynth::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    float* out1 = outputs[0];
    float* out2 = outputs[1];
    while (--sampleFrames >= 0) {
        (*out1++) = sin(phase);
        (*out2++) = sin(phase);
        phase += 3.14f * 440 / 44100;
    }
}
