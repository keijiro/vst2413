#ifndef __driver__
#define __driver__

#include <string>

extern "C" {
    struct __OPLL;
}

class Driver {
public:
    enum {
        kParameterAR0,
        kParameterAR1,
        kParameterDR0,
        kParameterDR1,
        kParameterSL0,
        kParameterSL1,
        kParameterRR0,
        kParameterRR1,
        kParameterMUL0,
        kParameterMUL1,
        kParameterMax
    };
    
    Driver(unsigned int sampleRate);
    ~Driver();
    
    void SetSampleRate(unsigned int sampleRate);
    void SetProgram(int number);
    const char* GetProgramName(int number);
    
    void KeyOn(int noteNumber, int velocity);
    void KeyOff(int noteNumber);
    void KeyOffAll();
    
    void SetParameter(int index, float value);
    float GetParameter(int index);
    const char* GetParameterName(int index);
    std::string GetParameterText(int index);
    
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
    struct __OPLL* opll_;
    float parameters_[kParameterMax];
    unsigned char dump_[16];
};

#endif
