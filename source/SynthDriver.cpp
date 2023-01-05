#include "SynthDriver.h"
#include "emu2413/emu2413.h"
#include <cmath>
#include <stdio.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

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
        int CalculateFNumber(int note, float tune) {
            int intervalFromA = (note - 9) % 12;
            return 144.1792f * powf(2.0f, (1.0f / 12) * (intervalFromA + tune));
        }
        
        int NoteToBlock(int note) {
            return Clamp((note - 9) / 12, 0, 7);
        }
        
        int CalculateBlockAndFNumber(int note, const float* parameters, float wheel) {
            int range = parameters[SynthDriver::kParameterWheelRange] * 12;
            float tune = parameters[SynthDriver::kParameterFineTune] - 0.5f;
            return (NoteToBlock(note) << 9) + CalculateFNumber(note, wheel * range + tune);
        }

        void SendKeyOn(OPLL* opll, const float* parameters, int channel, int program, int note, float wheel, float velocity) {
            int bf = CalculateBlockAndFNumber(note, parameters, wheel);
            int vl = 15.0f - velocity * 15;
            OPLL_writeReg(opll, 0x10 + channel, bf & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, 0x10 + (bf >> 8));
            OPLL_writeReg(opll, 0x30 + channel, (program << 4) + vl);
        }

        void SendKeyOff(OPLL* opll, const float* parameters, int channel, int program, int note, float wheel) {
            int bf = CalculateBlockAndFNumber(note, parameters, wheel);
            OPLL_writeReg(opll, 0x20 + channel, bf >> 8);
        }

        void AdjustPitch(OPLL* opll, const float* parameters, int channel, int note, float wheel, bool keyOn) {
            int bf = CalculateBlockAndFNumber(note, parameters, wheel);
            OPLL_writeReg(opll, 0x10 + channel, bf & 0xff);
            OPLL_writeReg(opll, 0x20 + channel, (keyOn ? 0x10 : 0) + (bf >> 8));
        }
        
        void SendARDR(OPLL* opll, const float* parameters, int op) {
            int ar = (1.0f - parameters[SynthDriver::kParameterAR0 + op]) * 15;
            int dr = (1.0f - parameters[SynthDriver::kParameterDR0 + op]) * 15;
            OPLL_writeReg(opll, 4 + op, (ar << 4) + dr);
        }

        void SendSLRR(OPLL* opll, const float* parameters, int op) {
            int sl = (1.0f - parameters[SynthDriver::kParameterSL0 + op]) * 15;
            int rr = (1.0f - parameters[SynthDriver::kParameterRR0 + op]) * 15;
            OPLL_writeReg(opll, 6 + op, (sl << 4) + rr);
        }

        void SendMUL(OPLL* opll, const float* parameters, int op) {
            int am  = parameters[SynthDriver::kParameterAM0  + op] < 0.5f ? 0 : 0x80;
            int vib = parameters[SynthDriver::kParameterVIB0 + op] < 0.5f ? 0 : 0x40;
            int mul = parameters[SynthDriver::kParameterMUL0 + op] * 15;
            OPLL_writeReg(opll, op, am + vib + 0x20 + mul);
        }

        void SendFB(OPLL* opll, const float* parameters) {
            int dc = parameters[SynthDriver::kParameterDC] < 0.5f ? 0 : 0x10;
            int dm = parameters[SynthDriver::kParameterDM] < 0.5f ? 0 : 0x08;
            int fb = parameters[SynthDriver::kParameterFB] * 7;
            OPLL_writeReg(opll, 3, dc + dm + fb);
        }

        void SendTL(OPLL* opll, const float* parameters) {
            int tl = (1.0f - parameters[SynthDriver::kParameterTL]) * 63;
            OPLL_writeReg(opll, 2, tl);
        }
    }
}

#pragma mark
#pragma mark Creation and destruction

SynthDriver::SynthDriver(unsigned int sampleRate)
:   opll_(0),
    program_(kProgramUser),
    lastChannel_(0),
    wheel_(0)
{
    opll_ = OPLL_new(kMasterClock, sampleRate);
    // Initialize all the parameters.
    for (int i = 0; i < kParameters; i++) {
        parameters_[i] = 0.0f;
    }
    parameters_[kParameterSL0] = 1.0f;
    parameters_[kParameterSL1] = 1.0f;
    parameters_[kParameterMUL0] = 1.1f / 15;
    parameters_[kParameterMUL1] = 1.1f / 15;
    parameters_[kParameterWheelRange] = 3.0f / 12;
    parameters_[kParameterFineTune] = 0.5f;
    // Initialize the program on the OPLL.
    OPLLC::SendARDR(opll_, parameters_, 0);
    OPLLC::SendARDR(opll_, parameters_, 1);
    OPLLC::SendSLRR(opll_, parameters_, 0);
    OPLLC::SendSLRR(opll_, parameters_, 1);
    OPLLC::SendMUL(opll_, parameters_, 0);
    OPLLC::SendMUL(opll_, parameters_, 1);
    OPLLC::SendFB(opll_, parameters_);
    OPLLC::SendTL(opll_, parameters_);
}

SynthDriver::~SynthDriver() {
    OPLL_delete(opll_);
}

#pragma mark
#pragma mark Output setting

void SynthDriver::SetSampleRate(unsigned int sampleRate) {
    OPLL_set_rate(opll_, sampleRate);
}

#pragma mark
#pragma mark Program

SynthDriver::String SynthDriver::GetProgramName(ProgramID id) {
    static const char* names[] = {
        "User",
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
        "S.Bass",
        "A.Bass",
        "E.Guitar"
    };
    return names[id];
}

#pragma mark
#pragma mark Key on and off

void SynthDriver::KeyOn(int note, float velocity) {
    int index = ChooseChannelIndex();
    ChannelInfo& info = channels_[index];
    OPLLC::SendKeyOn(opll_, parameters_, index, program_, note, wheel_, velocity);
    info.note_ = note;
    info.velocity_ = velocity;
    info.active_ = true;
    lastChannel_ = index;
}

void SynthDriver::KeyOff(int note) {
    for (int i = 0; i < kChannels; i++) {
        ChannelInfo& info = channels_[i];
        if (info.active_ && info.note_ == note) {
            OPLLC::SendKeyOff(opll_, parameters_, i, program_, note, wheel_);
            info.active_ = false;
            break;
        }
    }
}

void SynthDriver::KeyOffAll() {
    for (int i = 0; i < kChannels; i++) {
        ChannelInfo& info = channels_[i];
        if (info.active_) {
            OPLLC::SendKeyOff(opll_, parameters_, i, program_, info.note_, wheel_);
            info.active_ = false;
        }
    }
}

#pragma mark
#pragma mark Modifiers

void SynthDriver::SetPitchWheel(float value) {
    wheel_ = value;
    for (int i = 0; i < kChannels; i++) {
        ChannelInfo& info = channels_[i];
        OPLLC::AdjustPitch(opll_, parameters_, i, info.note_, wheel_, info.active_);
    }
}

#pragma mark
#pragma mark Parameters

void SynthDriver::SetParameter(ParameterID id, float value) {
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
        case kParameterWheelRange:
        case kParameterFineTune:
            SetPitchWheel(wheel_);
        case kParameters:
            break;
    }
}

float SynthDriver::GetParameter(ParameterID id) {
    return parameters_[id];
}

SynthDriver::String SynthDriver::GetParameterName(ParameterID id) {
    static const char* names[kParameters] = {
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
        "VIB1",
        "P.Wheel",
        "FineTune"
    };
    return names[id];
}

SynthDriver::String SynthDriver::GetParameterLabel(ParameterID id) {
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
        case kParameterWheelRange:
            return "st";
        case kParameterFineTune:
            return "cent";
        default:
            return "";
    }
}

SynthDriver::String SynthDriver::GetParameterText(ParameterID id) {
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
    // Wheel range
    if (id == kParameterWheelRange) {
        char buffer[32];
        snprintf(buffer, sizeof buffer, "%d", static_cast<int>(parameters_[id] * 12));
        return buffer;
    }
    // Fine tune
    if (id == kParameterFineTune) {
        char buffer[32];
        snprintf(buffer, sizeof buffer, "%.2f", (parameters_[id] - 0.5f) * 100);
        return buffer;
    }
    // Switches
    return parameters_[id] < 0.5f ? "off" : "on";
}

#pragma mark
#pragma mark Output processing

float SynthDriver::Step() {
    return (4.0f / 32767) * OPLL_calc(opll_);
}

#pragma mark
#pragma mark Internal functions

int SynthDriver::ChooseChannelIndex() {
    int index = lastChannel_;
    for (int offs = 0; offs < kChannels - 1; offs++) {
        if (++index == kChannels) index = 0;
        if (!channels_[index].active_) return index;
    }
    return lastChannel_ + 1;
}
