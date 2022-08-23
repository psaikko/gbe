#pragma once

#include "sound_defs.h"
#include <inttypes.h>
#include <unordered_map>

#define TCLK_HZ        4194304u
#define SAMPLE_RATE    44000u
#define SOUND_MEM_SIZE 48

class Sound {
  public:
    Sound();

    void update(unsigned tclk);

    bool hasNewSample();

    void getSamples(sample_t *left, sample_t *right);

    void writeByte(uint16_t addr, uint8_t val);
    uint8_t readByte(uint16_t addr);

    bool mute_ch1{false};
    bool mute_ch2{false};
    bool mute_ch3{false};
    bool mute_ch4{false};

    unsigned long samples{0};

  private:
    bool sample_ready{false};
    unsigned clock{0};
    sample_t lsample{0}, rsample{0};
    int8_t wave_pattern_ram[16];
    sample_t sample_map[16];
    sample_t square_map[33];

    struct CH1;
    struct CH2;
    struct CH3;
    struct CH4;
    struct CTRL;

    CH1 *Channel1;
    CH2 *Channel2;
    CH3 *Channel3;
    CH4 *Channel4;
    CTRL *Control;

    int internal_256hz_counter{TCLK_HZ / 256};

    std::unordered_map<uint16_t, uint8_t *> reg_pointers;
    std::unordered_map<uint16_t, uint8_t> reg_masks;

    sample_t updateCh1(unsigned tclock, bool length_tick);
    sample_t updateCh2(unsigned tclock, bool length_tick);
    sample_t updateCh3(unsigned tclock, bool length_tick);
    sample_t updateCh4(unsigned tclock, bool length_tick);

    void clearRegisters();
};

// 1 step = n/64 sec