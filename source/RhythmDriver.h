#ifndef __RhythmDriver__
#define __RhythmDriver__

#include <string>

extern "C" {
    struct __OPLL;
}

class RhythmDriver {
public:
    typedef std::string String;
    
    RhythmDriver(unsigned int sampleRate);
    ~RhythmDriver();
    
    void SetSampleRate(unsigned int sampleRate);
    
    void KeyOn(int note, float velocity);
    void KeyOff(int note);
    void KeyOffAll();
    
    float Step();
    
private:
    struct __OPLL* opll_;
    int state_;
    float volumes_[6];
};

#endif
