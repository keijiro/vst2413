#include "Vst2413.h"

VstInt32 Vst2413::processEvents(VstEvents* events) {
	for (VstInt32 i = 0; i < events->numEvents; i++) {
		if ((events->events[i])->type != kVstMidiType) continue;
        
		const VstMidiEvent* ev = reinterpret_cast<VstMidiEvent*>(events->events[i]);
		const char* midiData = ev->midiData;
		VstInt32 status = midiData[0] & 0xf0;
        
        if (status == 0x90) {
            driver_.KeyOn(midiData[1] & 0x7f, 1.0f / 128 * (midiData[2] & 0x7f));
        } else if (status == 0x80) {
            driver_.KeyOff(midiData[1] & 0x7f);
        } else if (status == 0xb0) {
            if (midiData[1] == 0x7e || midiData[1] == 0x7b) {
                driver_.KeyOffAll();
            }
        }
	}
	return 1;
}

void Vst2413::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    for (VstInt32 i = 0; i < sampleFrames; i++) outputs[0][i] = driver_.Step();
}
