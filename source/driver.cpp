#include "driver.h"
#include "emu2413.h"

namespace {
    const unsigned int kMsxClock = 3579540;
    
    namespace OPLLC {
        void SendKey(OPLL *opll, int channel, int noteNumber, bool keyOn) {
            static const int fnums[] = { 181, 192, 204, 216, 229, 242, 257, 272, 288, 305, 323, 343 };
            int fnum = fnums[(noteNumber - 37) % 12];
            int block = (noteNumber - 37) / 12;
            block = block < 0 ? 0 : (block > 7) ? 7 : block;
            OPLL_writeReg(opll, 0x10 + channel, fnum & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (fnum >> 8) + (block << 1));
        }
        
        void SendARDR(OPLL* opll, float* parameters, int op) {
            unsigned int data =
                (static_cast<unsigned int>((1.0f - parameters[Driver::kParameterAR0 + op]) * 255) & 0xf0) +
                 static_cast<unsigned int>((1.0f - parameters[Driver::kParameterDR0 + op]) * 15);
            OPLL_writeReg(opll, 4 + op, data);
        }

        void SendSLRR(OPLL* opll, float* parameters, int op) {
            unsigned int data =
                (static_cast<unsigned int>((1.0f - parameters[Driver::kParameterSL0 + op]) * 255) & 0xf0) +
                 static_cast<unsigned int>((1.0f - parameters[Driver::kParameterRR0 + op]) * 15);
            OPLL_writeReg(opll, 6 + op, data);
        }

        void SendMUL(OPLL* opll, float* parameters, int op) {
            unsigned int data =
                0x20 +
                static_cast<unsigned int>(parameters[Driver::kParameterMUL0 + op] * 15);
            OPLL_writeReg(opll, op, data);
        }
    }
}

Driver::Driver(unsigned int sampleRate)
:   sampleRate_(sampleRate),
    opll_(0)
{
    opll_ = OPLL_new(kMsxClock, sampleRate_);
    for (int i = 0; i < kParameterMax; i++) {
        parameters_[i] = 0.0f;
    }
}

Driver::~Driver() {
    OPLL_delete(opll_);
}

void Driver::SetSampleRate(unsigned int sampleRate) {
    sampleRate_ = sampleRate;
    OPLL_set_rate(opll_, sampleRate_);
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
            OPLLC::SendKey(opll_, i, noteNumber, true);
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
            OPLLC::SendKey(opll_, i, noteNumber, false);
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
    switch (index) {
        case kParameterAR0:
        case kParameterDR0:
            OPLLC::SendARDR(opll_, parameters_, 0);
            break;
        case kParameterAR1:
        case kParameterDR1:
            OPLLC::SendARDR(opll_, parameters_, 1);
            break;
        case kParameterSL0:
        case kParameterRR0:
            OPLLC::SendSLRR(opll_, parameters_, 0);
            break;
        case kParameterSL1:
        case kParameterRR1:
            OPLLC::SendSLRR(opll_, parameters_, 1);
            break;
        case kParameterMUL0:
            OPLLC::SendMUL(opll_, parameters_, 0);
            break;
        case kParameterMUL1:
            OPLLC::SendMUL(opll_, parameters_, 1);
            break;
    }
}

float Driver::GetParameter(int index) {
    return parameters_[index];
}

const char* Driver::GetParameterName(int index) {
    static const char *names[kParameterMax] = {
        "AR 0",
        "AR 1",
        "DR 0",
        "DR 1",
        "SL 0",
        "SL 1",
        "RR 0",
        "RR 1",
        "MUL 0",
        "MUL 1"
    };
    return names[index];
}

std::string Driver::GetParameterText(int index) {
    char buffer[32];
    snprintf(buffer, sizeof buffer, "%.1f", parameters_[index]);
    return std::string(buffer);
}

float Driver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
