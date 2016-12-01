#include "sound.h"
#include <cassert>

struct Sound::CH1 {
	enum op { Addition, Subtraction };

	struct { // rw
		uint8_t sweep_number : 3; // 0-7
		op      sweep_mode   : 1; 
		uint8_t sweep_time   : 3; // x / 128 Hz
		uint8_t _            : 1; 
	} NR10;

	struct { // rw
		uint8_t sound_length : 6; // len = (64 - t)*(1/256) sec
		uint8_t wave_duty    : 2; // 12.5%, 25%, 50%, 75%
	} NR11;

	struct { // rw
		uint8_t env_sweep       : 3; // 0 = stop
		direction env_direction : 1;
		uint8_t env_volume      : 4; // n: 0 = no sound
	} NR12;

	struct {
		uint8_t freq_lo; // wo
	} NR13;

	struct {
		uint8_t freq_hi     : 3; // wo
		uint8_t _           : 3;
		uint8_t consecutive : 1; // rw
		uint8_t init        : 1; // wo
	} NR14;
};

struct Sound::CH2 {

	struct {
		uint8_t sound_length : 6; // wo, len = (64 - t)*(1/256) sec
		uint8_t wave_duty    : 2; // rw, 12.5%, 25%, 50%, 75%
	} NR21;		

	struct { // rw
		uint8_t env_sweep       : 3; // 0 = stop
		direction env_direction : 1;
		uint8_t env_volume      : 4; // n: 0 = no sound
	} NR22;

	struct {
		uint8_t freq_lo; // wo
	} NR23;

	struct {
		uint8_t freq_hi     : 3; // wo
		uint8_t _           : 3;
		uint8_t consecutive : 1; // rw
		uint8_t init        : 1; // wo
	} NR24;
};

struct Sound::CH3 {
	struct {
		uint8_t _        : 7;
		uint8_t sound_on : 1; // rw
	} NR30;

	struct {
		uint8_t sound_length;
	} NR31;

	struct {
		uint8_t _      : 4;
		uint8_t volume : 2; // rw
		uint8_t __     : 4;
		// 0=0% 1=100% 2=50% 3=25%
	} NR32;

	struct { // wo
		uint8_t freq_lo;
	} NR33;

	struct {
		uint8_t freq_hi     : 3; // wo
		uint8_t _           : 3;
		uint8_t consecutive : 1; // rw
		uint8_t init        : 1; // wo
	} NR34;

	// Wave Pattern RAM: FF30 - FF3F
};

struct Sound::CH4 {
	struct {
		uint8_t length : 6;
		uint8_t _ : 2;
	} NR41;

	struct {
		uint8_t env_sweep       : 3; // 0 = stop
		direction env_direction : 1;
		uint8_t env_volume      : 4; // n: 0 = no sound
	} NR42;

	struct {
		uint8_t freq_div : 3; // r
		uint8_t counter_step : 1; // 0=15 bits, 1=7 bits
		uint8_t shift_clk_freq : 4; // s
		// freq = 524288 / r / 2^(s+1) Hz  (assume r=0.5 when 0)
	} NR43;

	struct {
		uint8_t _           : 6;
		uint8_t consecutive : 1; // rw
		uint8_t init        : 1; // wo
	} NR44;
};

struct Sound::CTRL {
	struct { // rw
		uint8_t SO1_vol : 3;
		uint8_t SO1_vin : 1;
		uint8_t SO2_vol : 3;
		uint8_t SO2_vin : 1;
	} NR50;

	struct { // rw
		uint8_t CH1_SO1 : 1;
		uint8_t CH2_SO1 : 1;
		uint8_t CH3_SO1 : 1;
		uint8_t CH4_SO1 : 1;
		uint8_t CH1_SO2 : 1;
		uint8_t CH2_SO2 : 1;
		uint8_t CH3_SO2 : 1;
		uint8_t CH4_SO2 : 1;
	} NR51;

	struct {
		uint8_t CH1_on   : 1; // ro
		uint8_t CH2_on   : 1; // ro
		uint8_t CH3_on   : 1; // ro
		uint8_t CH4_on   : 1; // ro
		uint8_t _        : 3;
		uint8_t sound_on : 1; // rw
	} NR52;
};

Sound::Sound() : clock(0), lsample(0), rsample(0) {
	Channel1 = new CH1();
	Channel2 = new CH2();
	Channel3 = new CH3();
	Channel4 = new CH4();
	Control  = new CTRL();

	reg_pointers = {
		{0xFF10, (uint8_t*)&(Channel1->NR10)},
		{0xFF11, (uint8_t*)&(Channel1->NR11)},
		{0xFF12, (uint8_t*)&(Channel1->NR12)},
		{0xFF13, (uint8_t*)&(Channel1->NR13)},
		{0xFF14, (uint8_t*)&(Channel1->NR14)},

		{0xFF16, (uint8_t*)&(Channel2->NR21)},
		{0xFF17, (uint8_t*)&(Channel2->NR22)},
		{0xFF18, (uint8_t*)&(Channel2->NR23)},
		{0xFF19, (uint8_t*)&(Channel2->NR24)},

		{0xFF1A, (uint8_t*)&(Channel3->NR30)},
		{0xFF1B, (uint8_t*)&(Channel3->NR31)},
		{0xFF1C, (uint8_t*)&(Channel3->NR32)},
		{0xFF1D, (uint8_t*)&(Channel3->NR33)},
		{0xFF1E, (uint8_t*)&(Channel3->NR34)},

		{0xFF20, (uint8_t*)&(Channel4->NR41)},
		{0xFF21, (uint8_t*)&(Channel4->NR42)},
		{0xFF22, (uint8_t*)&(Channel4->NR43)},
		{0xFF23, (uint8_t*)&(Channel4->NR44)},

		{0xFF24, (uint8_t*)&(Control->NR50)},
		{0xFF25, (uint8_t*)&(Control->NR51)},
		{0xFF26, (uint8_t*)&(Control->NR52)}
	};
}

void Sound::tick(unsigned tclk) {
	clock += tclk;
}

bool Sound::hasNewSample() {
	return true;
}

void Sound::writeByte(uint16_t addr, uint8_t val) {
	// mask out readonly bits of FF26
	if (addr == 0xFF26) val &= 0x80;
	assert(reg_pointers.count(addr));
	*reg_pointers[addr] = val;
}

uint8_t Sound::readByte(uint16_t addr) {
	// mask out writeonly bits 
	uint8_t mask = 0;
	switch (addr) {
		case 0xFF11:
			mask = 0x1F;
			break;
		case 0xFF13:
			mask = 0xFF;
			break;
		case 0xFF14:
			mask = 0x83;
			break;
		case 0xFF16:
			mask = 0x1F;
			break;
		case 0xFF18:
			mask = 0xFF;
			break;
		case 0xFF19:
			mask = 0x83; 
			break;
		case 0xFF1D:
			mask = 0xFF;
			break;
		case 0xFF1E:
			mask = 0x83;
			break;
		default:
			break;
	}
	mask = ~mask;
	assert(reg_pointers.count(addr));
	uint8_t val = *reg_pointers[addr];
	return val & mask;
}

void Sound::getSamples(uint8_t * left, uint8_t * right) {
	*left = 0;
	*right = 0;
}

void Sound::updateCh1() {

}

void Sound::updateCh2() {
	
}

void Sound::updateCh3() {
	
}

void Sound::updateCh4() {
	
}