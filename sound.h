#pragma once

#include <inttypes.h>
#include <unordered_map>
#include "sound_defs.h"

#define TCLK_HZ   4194304
#define SAMPLE_RATE 44000

class Sound {
public:
	Sound();

	void update(unsigned tclk);

	bool hasNewSample();

	void getSamples(sample_t *left, sample_t *right);

	void writeByte(uint16_t addr, uint8_t val);
	uint8_t readByte(uint16_t addr);

	bool mute_ch1;
	bool mute_ch2;
	bool mute_ch3;
	bool mute_ch4;

private:
	bool sample_ready;
	unsigned clock;
	sample_t lsample, rsample;
	int8_t wave_pattern_ram[16];
	sample_t sample_map[16];
	sample_t square_map[33];

	enum direction { Decrease, Increase };

	struct CH1;
	struct CH2;
	struct CH3;
	struct CH4;
	struct CTRL;

	CH1* Channel1;
	CH2* Channel2;
	CH3* Channel3;
	CH4* Channel4;
	CTRL* Control;

	std::unordered_map<uint16_t, uint8_t*> reg_pointers;

	sample_t updateCh1();
	sample_t updateCh2();
	sample_t updateCh3(unsigned tclock);
	sample_t updateCh4();
};

// 1 step = n/64 sec