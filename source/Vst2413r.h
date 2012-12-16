#ifndef __Vst2413r__
#define __Vst2413r__

#include "audioeffectx.h"
#include "RhythmDriver.h"

class Vst2413r : public AudioEffectX {
public:
    static const unsigned long kUniqueId = 'dAzz';

    Vst2413r(audioMasterCallback audioMaster);
    ~Vst2413r();

	virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents(VstEvents* events);
	
	virtual void setSampleRate(float sampleRate);
	virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
    
	virtual bool getEffectName(char* name);
	virtual bool getVendorString(char* text);
	virtual bool getProductString(char* text);
	virtual VstInt32 getVendorVersion();
	virtual VstInt32 canDo(char* text);
    
	virtual VstInt32 getNumMidiInputChannels();
	virtual VstInt32 getNumMidiOutputChannels();

private:
    RhythmDriver driver_;
};

#endif
