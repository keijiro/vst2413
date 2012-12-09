#include "driver.h"
#include "emu2413.h"
#include <cmath>

namespace {
    const unsigned int kMsxClock = 3579540;
    
    template <typename T> T Clamp(T value, T min, T max) {
        return value < min ? min : (value > max ? max : value);
    }
    
    namespace OPLLC {
        int CalcFNum(int note) {
            int intervalFromA = (note - 21) % 12;
            return 144.1792f * powf(2.0f, (1.0f / 12) * intervalFromA);
        }
        
        int CalcBlock(int note) {
            return Clamp((note - 9) / 12, 0, 7);
        }

        void SendKey(OPLL *opll, int channel, int program, int noteNumber, float velocity, bool keyOn) {
            int fnum = CalcFNum(noteNumber);
            int block = CalcBlock(noteNumber);
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (fnum >> 8) + (block << 1));
            if (keyOn) {
                OPLL_writeReg(opll, 0x10 + channel, fnum & 0xff);
                OPLL_writeReg(opll, 0x30 + channel, (program << 4) + static_cast<int>(15.0f - velocity * 15));
            }
        }
        
        void AdjustPitch(OPLL* opll, int channel, int noteNumber, float velocity, bool keyOn) {
            int fnum = CalcFNum(noteNumber);
            int block = CalcBlock(noteNumber);
            block = block < 0 ? 0 : (block > 7) ? 7 : block;
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (fnum >> 8) + (block << 1));
            OPLL_writeReg(opll, 0x10 + channel, fnum & 0xff);
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
    opll_(0),
    program_(0)
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
    program_ = number;
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

void Driver::KeyOn(int noteNumber, float velocity) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        if (!note.active_) {
            OPLLC::SendKey(opll_, i, program_, noteNumber, velocity, true);
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
            OPLLC::SendKey(opll_, i, program_, noteNumber, 0, false);
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

void Driver::SetPitchWheel(float value) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        OPLLC::AdjustPitch(opll_, i, note.noteNumber_, value, note.active_);
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

const char* Driver::GetParameterLabel(int index) {
    switch (index) {
        case kParamAR0:
        case kParamAR1:
        case kParamDR0:
        case kParamDR1:
        case kParamRR0:
        case kParamRR1:
            return "msec";
        case kParamSL0:
        case kParamSL1:
        case kParamTL:
            return "db";
    }
    return "";
}

std::string Driver::GetParameterText(int index) {
    // Attack rates
    if (index == kParamAR0 || index == kParamAR1) {
        static const char* texts[16] = {
            "0", "0.28", "0.50", "0.84",
            "1.69", "3.30", "6.76", "13.52",
            "27.03", "54.87", "108.13", "216.27",
            "432.54", "865.88", "1730.15", "inf"
        };
        return texts[static_cast<int>(parameters_[index] * 15)];
    }
    // Decay-like rates
    if (index == kParamDR0 || index == kParamDR1 || index == kParamRR0 || index == kParamRR1) {
        static const char* texts[16] = {
            "1.27", "2.55", "5.11", "10.22",
            "20.44", "40.07", "81.74", "163.49",
            "326.98", "653.95", "1307.91", "2615.82",
            "5231.64", "10463.30", "20926.60", "inf"
        };
        return texts[static_cast<int>(parameters_[index] * 15)];
    }
    // Levels
    if (index == kParamSL0 || index == kParamSL1 || index == kParamTL) {
        char buffer[32];
        snprintf(buffer, sizeof buffer, "%d", static_cast<int>((1.0f - parameters_[index]) * 45));
        return std::string(buffer);
    }
    // Multipliers
    if (index == kParamMUL0 || index == kParamMUL1) {
        static const char* texts[16] = {
            "1/2", "1", "2", "3",
            "4", "5", "6", "7",
            "8", "9", "10", "10",
            "12", "12", "15", "15"
        };
        return texts[static_cast<int>(parameters_[index] * 15)];
    }
    // Feedback
    if (index == kParamFB) {
        static const char* texts[8] = {
            "0", "n/16", "n/8", "n/4", "n/2", "n", "2n", "4n"
        };
        return texts[static_cast<int>(parameters_[index] * 7)];
    }
    // Switches
    return parameters_[index] < 0.5f ? "off" : "on";
}

float Driver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
