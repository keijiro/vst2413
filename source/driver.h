#ifndef __driver__
#define __driver__

#include <string>

extern "C" {
    struct __OPLL;
}

class Driver {
public:
    enum {
        kParamAR0,
        kParamAR1,
        kParamDR0,
        kParamDR1,
        kParamSL0,
        kParamSL1,
        kParamRR0,
        kParamRR1,
        kParamMUL0,
        kParamMUL1,
        kParamFB,
        kParamTL,
        kParamDM,
        kParamDC,
        kParamAM0,
        kParamAM1,
        kParamVIB0,
        kParamVIB1,
        kParamMax
    };
    
    Driver(unsigned int sampleRate);
    ~Driver();
    
    void SetSampleRate(unsigned int sampleRate);
    void SetProgram(int number);
    const char* GetProgramName(int number);
    
    void KeyOn(int noteNumber, float velocity);
    void KeyOff(int noteNumber);
    void KeyOffAll();
    void SetPitchWheel(float value);
    
    void SetParameter(int index, float value);
    float GetParameter(int index);
    const char* GetParameterName(int index);
    const char* GetParameterLabel(int index);
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
    int program_;
    float parameters_[kParamMax];
};

#endif
