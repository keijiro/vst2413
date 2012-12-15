#ifndef __driver__
#define __driver__

#include <string>

extern "C" {
    struct __OPLL;
}

class Driver {
public:
    typedef std::string String;
    
    enum ProgramID {
        kProgramUser,
        kProgramMax = 16
    };
    
    enum ParameterID {
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
        kParameterFB,
        kParameterTL,
        kParameterDM,
        kParameterDC,
        kParameterAM0,
        kParameterAM1,
        kParameterVIB0,
        kParameterVIB1,
        kParameterMax
    };
    
    Driver(unsigned int sampleRate);
    ~Driver();
    
    void SetSampleRate(unsigned int sampleRate);
    void SetProgram(ProgramID id);
    ProgramID GetProgram() { return program_; }
    String GetProgramName(ProgramID id);
    
    void KeyOn(int noteNumber, float velocity);
    void KeyOff(int noteNumber);
    void KeyOffAll();
    void SetPitchWheel(float value);
    
    void SetParameter(ParameterID id, float value);
    float GetParameter(ParameterID id);
    String GetParameterName(ParameterID id);
    String GetParameterLabel(ParameterID id);
    String GetParameterText(ParameterID id);
    
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
    ProgramID program_;
    float pitchWheel_;
    float parameters_[kParameterMax];
};

#endif
