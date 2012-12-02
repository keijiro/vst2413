#ifndef __driver__
#define __driver__

extern "C" {
    struct OPLL;
}

class Driver {
public:
    Driver(unsigned int sampleRate);
    ~Driver();
    
    void SetSampleRate(unsigned int sampleRate);
    void SetProgram(int number);
    const char* GetProgramName(int number);
    
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
