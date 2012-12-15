#include "driver.h"
#include "emu2413.h"
#include <cmath>

namespace {
    // OPLL master clock = 3.579545 MHz
    const unsigned int kMasterClock = 3579545;
    
#pragma mark Utility functions
    
    template <typename T> T Clamp(T value, T min, T max) {
        return value < min ? min : (value > max ? max : value);
    }

#pragma mark
#pragma mark OPLL controller functions

    namespace OPLLC {
        int CalculateFNumber(int note, float wheel) {
            int intervalFromA = (note - 9) % 12;
            return 144.1792f * powf(2.0f, (1.0f / 12) * (intervalFromA + wheel * 6));
        }
        
        int NoteToBlock(int note) {
            return Clamp((note - 9) / 12, 0, 7);
        }
        
        int CalculateBlockAndFNumber(int note, float wheel) {
            return (NoteToBlock(note) << 9) + CalculateFNumber(note, wheel);
        }

        void SendKey(OPLL* opll, int channel, int program, int note, float wheel, float velocity, bool keyOn) {
            int bf = CalculateBlockAndFNumber(note, wheel);
            int vl = 15.0f - velocity * 15;
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (bf >> 8));
            if (keyOn) {
                OPLL_writeReg(opll, 0x10 + channel, bf & 0xff);
                OPLL_writeReg(opll, 0x30 + channel, (program << 4) + vl);
            }
        }
        
        void AdjustPitch(OPLL* opll, int channel, int note, float wheel, bool keyOn) {
            int bf = CalculateBlockAndFNumber(note, wheel);
            OPLL_writeReg(opll, 0x10 + channel, bf & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (bf >> 8));
        }
        
        void SendARDR(OPLL* opll, const float* parameters, int op) {
            int ar = (1.0f - parameters[Driver::kParameterAR0 + op]) * 15;
            int dr = (1.0f - parameters[Driver::kParameterDR0 + op]) * 15;
            OPLL_writeReg(opll, 4 + op, (ar << 4) + dr);
        }

        void SendSLRR(OPLL* opll, const float* parameters, int op) {
            int sl = (1.0f - parameters[Driver::kParameterSL0 + op]) * 15;
            int rr = (1.0f - parameters[Driver::kParameterRR0 + op]) * 15;
            OPLL_writeReg(opll, 6 + op, (sl << 4) + rr);
        }

        void SendMUL(OPLL* opll, const float* parameters, int op) {
            int am  = parameters[Driver::kParameterAM0  + op] < 0.5f ? 0 : 0x80;
            int vib = parameters[Driver::kParameterVIB0 + op] < 0.5f ? 0 : 0x40;
            int mul = parameters[Driver::kParameterMUL0 + op] * 15;
            OPLL_writeReg(opll, op, am + vib + 0x20 + mul);
        }

        void SendFB(OPLL* opll, const float* parameters) {
            int dc = parameters[Driver::kParameterDC] < 0.5f ? 0 : 0x10;
            int dm = parameters[Driver::kParameterDM] < 0.5f ? 0 : 0x08;
            int fb = parameters[Driver::kParameterFB] * 7;
            OPLL_writeReg(opll, 3, dc + dm + fb);
        }

        void SendTL(OPLL* opll, const float* parameters) {
            int tl = (1.0f - parameters[Driver::kParameterTL]) * 63;
            OPLL_writeReg(opll, 2, tl);
        }
    }
}

#pragma mark
#pragma mark Creation and destruction

Driver::Driver(unsigned int sampleRate)
:   opll_(0),
    program_(kProgramUser),
    pitchWheel_(0)
{
    opll_ = OPLL_new(kMasterClock, sampleRate);

    for (int i = 0; i < kParameters; i++) {
        parameters_[i] = 0.0f;
    }
    
    parameters_[kParameterSL0] = 1.0f;
    parameters_[kParameterSL1] = 1.0f;
    parameters_[kParameterMUL0] = 1.1f / 15;
    parameters_[kParameterMUL1] = 1.1f / 15;
    
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
    OPLL_set_rate(opll_, sampleRate);
}

void Driver::SetProgram(ProgramID id) {
    program_ = id;
}

Driver::String Driver::GetProgramName(ProgramID id) {
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
    return names[id];
}

void Driver::KeyOn(int noteNumber, float velocity) {
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        if (!note.active_) {
            OPLLC::SendKey(opll_, i, program_, noteNumber, pitchWheel_, velocity, true);
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
            OPLLC::SendKey(opll_, i, program_, noteNumber, pitchWheel_, 0, false);
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
    pitchWheel_ = value;
    for (int i = 0; i < 9; i++) {
        NoteInfo& note = notes_[i];
        OPLLC::AdjustPitch(opll_, i, note.noteNumber_, pitchWheel_, note.active_);
    }
}

void Driver::SetParameter(ParameterID id, float value) {
    parameters_[id] = value;
    switch (id) {
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
        case kParameterVIB0:
        case kParameterAM0:
            OPLLC::SendMUL(opll_, parameters_, 0);
            break;
        case kParameterMUL1:
        case kParameterVIB1:
        case kParameterAM1:
            OPLLC::SendMUL(opll_, parameters_, 1);
            break;
        case kParameterFB:
        case kParameterDM:
        case kParameterDC:
            OPLLC::SendFB(opll_, parameters_);
            break;
        case kParameterTL:
            OPLLC::SendTL(opll_, parameters_);
            break;
        default:
            break;
    }
}

float Driver::GetParameter(ParameterID id) {
    return parameters_[id];
}

Driver::String Driver::GetParameterName(ParameterID id) {
    static const char *names[kParameters] = {
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
    return names[id];
}

Driver::String Driver::GetParameterLabel(ParameterID id) {
    switch (id) {
        case kParameterAR0:
        case kParameterAR1:
        case kParameterDR0:
        case kParameterDR1:
        case kParameterRR0:
        case kParameterRR1:
            return "msec";
        case kParameterSL0:
        case kParameterSL1:
        case kParameterTL:
            return "db";
        default:
            return "";
    }
}

Driver::String Driver::GetParameterText(ParameterID id) {
    // Attack rates
    if (id == kParameterAR0 || id == kParameterAR1) {
        static const char* texts[16] = {
            "0", "0.28", "0.50", "0.84",
            "1.69", "3.30", "6.76", "13.52",
            "27.03", "54.87", "108.13", "216.27",
            "432.54", "865.88", "1730.15", "inf"
        };
        return texts[static_cast<int>(parameters_[id] * 15)];
    }
    // Decay-like rates
    if (id == kParameterDR0 || id == kParameterDR1 || id == kParameterRR0 || id == kParameterRR1) {
        static const char* texts[16] = {
            "1.27", "2.55", "5.11", "10.22",
            "20.44", "40.07", "81.74", "163.49",
            "326.98", "653.95", "1307.91", "2615.82",
            "5231.64", "10463.30", "20926.60", "inf"
        };
        return texts[static_cast<int>(parameters_[id] * 15)];
    }
    // Levels
    if (id == kParameterSL0 || id == kParameterSL1 || id == kParameterTL) {
        char buffer[32];
        snprintf(buffer, sizeof buffer, "%d", static_cast<int>((1.0f - parameters_[id]) * 45));
        return buffer;
    }
    // Multipliers
    if (id == kParameterMUL0 || id == kParameterMUL1) {
        static const char* texts[16] = {
            "1/2", "1", "2", "3",
            "4", "5", "6", "7",
            "8", "9", "10", "10",
            "12", "12", "15", "15"
        };
        return texts[static_cast<int>(parameters_[id] * 15)];
    }
    // Feedback
    if (id == kParameterFB) {
        static const char* texts[8] = {
            "0", "n/16", "n/8", "n/4", "n/2", "n", "2n", "4n"
        };
        return texts[static_cast<int>(parameters_[id] * 7)];
    }
    // Switches
    return parameters_[id] < 0.5f ? "off" : "on";
}

float Driver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}
