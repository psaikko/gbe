#pragma once

#include <inttypes.h>
#include <unordered_map>

#define SAMPLE_RATE 44100

class Sound {
public:
	Sound();

	void tick(unsigned tclk);

	bool hasNewSample();

	void getSamples(uint8_t *left, uint8_t *right);

	void writeByte(uint16_t addr, uint8_t val);
	uint8_t readByte(uint16_t addr);

private:
	unsigned clock;
	int8_t lsample, rsample;

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

	void updateCh1();
	void updateCh2();
	void updateCh3();
	void updateCh4();
};

// 1 step = n/64 sec