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
    }
}

#pragma mark
#pragma mark Creation and destruction

RhythmDriver::RhythmDriver(unsigned int sampleRate)
:   opll_(0),
    state(0)
{
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
    state |= 1 << ((note - 24) % 12);
    OPLL_writeReg(opll_, 0xe, 0x20 + (state & 0x1f));
}

void RhythmDriver::KeyOff(int note) {
    state &= ~(1 << ((note - 24) % 12));
    OPLL_writeReg(opll_, 0xe, 0x20 + (state & 0x1f));
}

void RhythmDriver::KeyOffAll() {
    state = 0;
    OPLL_writeReg(opll_, 0xe, 0x20 + (state & 0x1f));
}

#pragma mark
#pragma mark Output processing

float RhythmDriver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
