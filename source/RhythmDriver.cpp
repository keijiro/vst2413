#include "RhythmDriver.h"
#include "emu2413.h"
#include <cmath>

namespace {
    // OPLL master clock = 3.579545 MHz
    const unsigned int kMasterClock = 3579545;
    
#pragma mark
#pragma mark OPLL controller functions

    namespace OPLLC {
        void ResetRhythmMode(OPLL* opll) {
            OPLL_writeReg(opll, 0x0e, 0x20);
            OPLL_writeReg(opll, 0x16, 0x20);
            OPLL_writeReg(opll, 0x17, 0x50);
            OPLL_writeReg(opll, 0x18, 0xC0);
            OPLL_writeReg(opll, 0x26, 0x05);
            OPLL_writeReg(opll, 0x27, 0x05);
            OPLL_writeReg(opll, 0x28, 0x01);
        }
        
        void SendKeyState(OPLL* opll, int state) {
            OPLL_writeReg(opll, 0xe, 0x20 + (state & 0x1f));
        }
        
        void SetVolumeRegisters(OPLL* opll, const float* volumes, int position) {
            int data1 = (1.0f - volumes[position & 6]) * 15;
            int data2 = (1.0f - volumes[position | 1]) * 15;
            OPLL_writeReg(opll, 0x36 + position / 2, data1 + (data2 << 4));
        }
    }

#pragma mark
#pragma mark Utility functions
    
    int NoteToKeyBit(int note) {
        switch (note) {
            case 36:
                return 16; // kick
            case 38:
                return 8; // snare
            case 43:
            case 47:
            case 50:
                return 4; // tom
            case 49:
            case 51:
                return 2; // cymbal
            case 42:
            case 44:
                return 1; // hat
        }
        return 0;
    }
    
    int NoteToVolumeRegisterPosition(int note) {
        switch (note) {
            case 36:
                return 0; // kick
            case 38:
                return 2; // snare
            case 43:
            case 47:
            case 50:
                return 5; // tom
            case 49:
            case 51:
                return 4; // cymbal
            case 42:
            case 44:
                return 3; // hat
        }
        return 1; // null
    }
}

#pragma mark
#pragma mark Creation and destruction

RhythmDriver::RhythmDriver(unsigned int sampleRate)
:   opll_(0),
    state_(0)
{
    for (int i = 0; i < 6; i++) volumes_[i] = 0;
    opll_ = OPLL_new(kMasterClock, sampleRate);
    OPLLC::ResetRhythmMode(opll_);
}

RhythmDriver::~RhythmDriver() {
    OPLL_delete(opll_);
}

#pragma mark
#pragma mark Output setting

void RhythmDriver::SetSampleRate(unsigned int sampleRate) {
    OPLL_set_rate(opll_, sampleRate);
}

#pragma mark
#pragma mark Key on and off

void RhythmDriver::KeyOn(int note, float velocity) {
    state_ |= NoteToKeyBit(note);
    int vrp = NoteToVolumeRegisterPosition(note);
    volumes_[vrp] = velocity;
    OPLLC::SetVolumeRegisters(opll_, volumes_, vrp);
    OPLLC::SendKeyState(opll_, state_);
}

void RhythmDriver::KeyOff(int note) {
    state_ &= ~NoteToKeyBit(note);
    OPLLC::SendKeyState(opll_, state_);
}

void RhythmDriver::KeyOffAll() {
    state_ = 0;
    OPLLC::SendKeyState(opll_, state_);
}

#pragma mark
#pragma mark Output processing

float RhythmDriver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
