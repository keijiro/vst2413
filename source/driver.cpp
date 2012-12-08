#include "driver.h"
#include "emu2413.h"

namespace {
    const unsigned int kMsxClock = 3579540;
    
    namespace OPLLC {
        void SendKey(OPLL *opll, int channel, int noteNumber, bool keyOn) {
            static const int fnums[] = { 181, 192, 204, 216, 229, 242, 257, 272, 288, 305, 323, 343 };
            int fnum = fnums[(noteNumber - 37) % 12];
            int block = (noteNumber - 13) / 12;
            block = block < 0 ? 0 : (block > 7) ? 7 : block;
            OPLL_writeReg(opll, 0x10 + channel, fnum & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (fnum >> 8) + (block << 1));
        }
        
        void SendARDR(OPLL* opll, float* parameters, int op) {
            unsigned int data =
                (static_cast<unsigned int>((1.0f - parameters[Driver::kParamAR0 + op]) * 255) & 0xf0) +
                 static_cast<unsigned int>((1.0f - parameters[Driver::kParamDR0 + op]) * 15);
            OPLL_writeReg(opll, 4 + op, data);
        }

        void SendSLRR(OPLL* opll, float* parameters, int op) {
            unsigned int data =
                (static_cast<unsigned int>((1.0f - parameters[Driver::kParamSL0 + op]) * 255) & 0xf0) +
                 static_cast<unsigned int>((1.0f - parameters[Driver::kParamRR0 + op]) * 15);
            OPLL_writeReg(opll, 6 + op, data);
        }

        void SendMUL(OPLL* opll, float* parameters, int op) {
            unsigned int data =
                (parameters[Driver::kParamAM0  + op] < 0.5f ? 0 : 0x80) +
                (parameters[Driver::kParamVIB0 + op] < 0.5f ? 0 : 0x40) +
                0x20 +
                static_cast<unsigned int>(parameters[Driver::kParamMUL0 + op] * 15);
            OPLL_writeReg(opll, op, data);
        }

        void SendFB(OPLL* opll, float* parameters) {
            unsigned int data =
                 (parameters[Driver::kParamDC] < 0.5f ? 0 : 0x10) +
                 (parameters[Driver::kParamDM] < 0.5f ? 0 : 0x08) +
                 static_cast<unsigned int>(parameters[Driver::kParamFB] * 7);
            OPLL_writeReg(opll, 3, data);
        }

        void SendTL(OPLL* opll, float* parameters) {
            unsigned int data =
                 static_cast<unsigned int>((1.0f - parameters[Driver::kParamTL]) * 63);
            OPLL_writeReg(opll, 2, data);
        }
    }
}

Driver::Driver(unsigned int sampleRate)
:   sampleRate_(sampleRate),
    opll_(0)
{
    opll_ = OPLL_new(kMsxClock, sampleRate_);

    for (int i = 0; i < kParamMax; i++) {
        parameters_[i] = 0.0f;
    }
    
    parameters_[kParamSL0] = 1.0f;
    parameters_[kParamSL1] = 1.0f;
    parameters_[kParamMUL0] = 1.1f / 15;
    parameters_[kParamMUL1] = 1.1f / 15;
    
    OPLLC::SendARDR(opll_, parameters_, 0);
    OPLLC::SendARDR(opll_, parameters_, 1);
    OPLLC::SendSLRR(opll_, parameters_, 0);
    OPLLC::SendSLRR(opll_, parameters_, 1);
    OPLLC::SendMUL(opll_, parameters_, 0);
    OPLLC::SendMUL(opll_, parameters_, 1);
    OPLLC::SendFB(opll_, parameters_);
    OPLLC::SendTL(opll_, parameters_);
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
        case kParamAR0:
        case kParamDR0:
            OPLLC::SendARDR(opll_, parameters_, 0);
            break;
        case kParamAR1:
        case kParamDR1:
            OPLLC::SendARDR(opll_, parameters_, 1);
            break;
        case kParamSL0:
        case kParamRR0:
            OPLLC::SendSLRR(opll_, parameters_, 0);
            break;
        case kParamSL1:
        case kParamRR1:
            OPLLC::SendSLRR(opll_, parameters_, 1);
            break;
        case kParamMUL0:
        case kParamVIB0:
        case kParamAM0:
            OPLLC::SendMUL(opll_, parameters_, 0);
            break;
        case kParamMUL1:
        case kParamVIB1:
        case kParamAM1:
            OPLLC::SendMUL(opll_, parameters_, 1);
            break;
        case kParamFB:
        case kParamDM:
        case kParamDC:
            OPLLC::SendFB(opll_, parameters_);
            break;
        case kParamTL:
            OPLLC::SendTL(opll_, parameters_);
            break;
    }
}

float Driver::GetParameter(int index) {
    return parameters_[index];
}

const char* Driver::GetParameterName(int index) {
    static const char *names[kParamMax] = {
        "AR 0",
        "AR 1",
        "DR 0",
        "DR 1",
        "SL 0",
        "SL 1",
        "RR 0",
        "RR 1",
        "MUL 0",
        "MUL 1",
        "FB",
        "TL",
        "DM",
        "DC",
        "AM0",
        "AM1",
        "VIB0",
        "VIB1"
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
