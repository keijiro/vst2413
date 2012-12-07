#include "driver.h"
#include "emu2413.h"

namespace {
    const unsigned int kMsxClock = 3579540;
    
    namespace OPLLC {
        void KeyOn(OPLL *opll, int channel, int noteNumber) {
            static const int fnums[] = { 181, 192, 204, 216, 229, 242, 257, 272, 288, 305, 323, 343 };
            int fnum = fnums[(noteNumber - 37) % 12];
            int block = (noteNumber - 37) / 12;
            block = block < 0 ? 0 : (block > 7) ? 7 : block;
            OPLL_writeReg(opll, 0x10 + channel, fnum & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, 0x30 + (fnum >> 8) + (block << 1));
        }
        
        void KeyOff(OPLL *opll, int channel) {
            OPLL_writeReg(opll, 0x20 + channel, 0);;
        }
    }
    
    int clamp(float value, int max) {
        return (value > 1.0f ? 1.0 : (value < 0.0f ? 0.0f : value)) * max;
    }
}

Driver::Driver(unsigned int sampleRate)
:   sampleRate_(sampleRate),
    opll_(0)
{
    opll_ = OPLL_new(kMsxClock, sampleRate_);
    // Reset with 3rd program.
    OPLL_PATCH patch[2];
    OPLL_getDefaultPatch(OPLL_2413_TONE, 3, patch);
    OPLL_patch2dump(patch, dump_);
    OPLL_setPatch(opll_, dump_);
}

Driver::~Driver() {
    OPLL_delete(opll_);
}

void Driver::SetSampleRate(unsigned int sampleRate) {
    sampleRate_ = sampleRate;
    OPLL_delete(opll_);
    opll_ = OPLL_new(kMsxClock, sampleRate_);
    OPLL_setPatch(opll_, dump_);
}

void Driver::SetProgram(int number) {
    int data = (number & 0xf) << 4;
    for (int i = 0; i < 9; i++) {
        OPLL_writeReg(opll_, 0x30 + i, data);
    }
}

const char* Driver::GetProgramName(int number) {
    static const char* names[] = {
        "User Program",
        "Violin",
        "Guitar",
        "Piano",
        "Flute",
        "Clarinet",
        "Oboe",
        "Trumpet",
        "Organ",
        "Horn",
        "Synthesizer",
        "Harpsichord",
        "Vibraphone",
        "Synthesizer Bass",
        "Acoustic Bass",
        "Electric Guitar"
    };
    return names[number & 0xf];
}

void Driver::KeyOn(int noteNumber, int velocity) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        if (!note.active_) {
            OPLLC::KeyOn(opll_, i, noteNumber);
            note.noteNumber_ = noteNumber;
            note.velocity_ = velocity;
            note.active_ = true;
            break;
        }
    }
}

void Driver::KeyOff(int noteNumber) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        if (note.active_ && note.noteNumber_ == noteNumber) {
            OPLLC::KeyOff(opll_, i);
            note.active_ = false;
            break;
        }
    }
}

void Driver::KeyOffAll() {
    for (int i = 0; i < 9; i++) {
        notes_[i].active_ = false;
    }
}

void Driver::SetParameter(int index, float value) {
    parameters_[index] = value;
}

float Driver::GetParameter(int index) {
    return parameters_[index];
}

const char* Driver::GetParameterName(int index) {
    static const char *names[kMaxParameterIndex] = {
        "AR 0",
        "AR 1",
        "DR 0",
        "DR 1",
        "SL 0",
        "SL 1",
        "RR 0",
        "RR 1",
        "MULTI 0",
        "MULTI 1"
    };
    return names[index];
}

std::string Driver::GetParameterText(int index) {
    char buffer[32];
    snprintf(buffer, sizeof buffer, "%.1f", parameters_[index]);
    return std::string(buffer);
}

#if 0
void Driver::SetAttack(int op, float value) {
    int addr = 4 + (op & 1);
    dump_[addr] = (dump_[addr] & 0xf) + (clamp(value, 15) << 4);
    OPLL_writeReg(opll_, addr, dump_[addr]);
}

void Driver::SetDecay(int op, float value) {
    int addr = 4 + (op & 1);
    dump_[addr] = (dump_[addr] & 0xf0) + clamp(value, 15);
    OPLL_writeReg(opll_, addr, dump_[addr]);
}

void Driver::SetSustain(int op, float value) {
    int addr = 5 + (op & 1);
    dump_[addr] = (dump_[addr] & 0xf) + (clamp(value, 15) << 4);
    OPLL_writeReg(opll_, addr, dump_[addr]);
}

void Driver::SetRelease(int op, float value) {
    int addr = 5 + (op & 1);
    dump_[addr] = (dump_[addr] & 0xf0) + clamp(value, 15);
    OPLL_writeReg(opll_, addr, dump_[addr]);
}

void Driver::SetMultiplier(int op, float value) {
    int addr = op & 1;
    dump_[addr] = (dump_[addr] & 0xf8) + clamp(value, 7);
    OPLL_writeReg(opll_, addr, dump_[addr]);
}
#endif

float Driver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
