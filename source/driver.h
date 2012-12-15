#ifndef __driver__
#define __driver__

#include <string>

extern "C" {
    struct __OPLL;
}

class Driver {
public:
    typedef std::string String;
    
    static const int kChannels = 9;
    
    enum ProgramID {
        kProgramUser,
        kPrograms = 16
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
        kParameters
    };
    
    Driver(unsigned int sampleRate);
    ~Driver();
    
    void SetSampleRate(unsigned int sampleRate);
    
    void SetProgram(ProgramID id) { program_ = id; }
    ProgramID GetProgram() { return program_; }
    String GetProgramName(ProgramID id);
    
    void KeyOn(int note, float velocity);
    void KeyOff(int note);
    void KeyOffAll();
    
    void SetPitchWheel(float value);
    
    void SetParameter(ParameterID id, float value);
    float GetParameter(ParameterID id);
    String GetParameterName(ParameterID id);
    String GetParameterLabel(ParameterID id);
    String GetParameterText(ParameterID id);
    
    float Step();
    
private:
    struct ChannelInfo {
        bool active_;
        int note_;
        int velocity_;
        ChannelInfo() : active_(false) {}
    };
    
    struct __OPLL* opll_;

    ProgramID program_;
    float parameters_[kParameters];
    
    ChannelInfo channels_[kChannels];
    float wheel_;
};

#endif
