#ifndef __SimpleSynth__SynthDriver__
#define __SimpleSynth__SynthDriver__

extern "C" {
    struct OPLL;
}

class SynthDriver {
public:
    SynthDriver(unsigned int sampleRate);
    ~SynthDriver();
    void SetSampleRate(unsigned int sampleRate);
    void KeyOn(int noteNumber, int velocity);
    void KeyOff(int noteNumber);
    void KeyOffAll();
    float Step();
    
private:
    struct NoteInfo {
        bool active_;
        int noteNumber_;
        int velocity_;
        
        NoteInfo() : active_(false) {}
    };
    
    unsigned int sampleRate_;
    NoteInfo notes_[9];
    struct OPLL* opll_;
};

#endif
