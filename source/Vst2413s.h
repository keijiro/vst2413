#ifndef __Vst2413s__
#define __Vst2413s__

#include "audioeffectx.h"
#include "SynthDriver.h"

class Vst2413s : public AudioEffectX {
public:
    static const unsigned long kUniqueId = 'dAzy';

    Vst2413s(audioMasterCallback audioMaster);
    ~Vst2413s();

	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);
    
	virtual void setProgram (VstInt32 program);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
    
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);
	
	virtual void setSampleRate (float sampleRate);
	virtual void setBlockSize (VstInt32 blockSize);
	
	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
    
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstInt32 canDo (char* text);
    
	virtual VstInt32 getNumMidiInputChannels ();
	virtual VstInt32 getNumMidiOutputChannels ();

private:
    SynthDriver driver_;
};

#endif
