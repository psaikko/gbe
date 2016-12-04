#include "sound.h"
#include <cassert>

struct Sound::CH1 {
	enum op { Addition, Subtraction };

	union {
		struct { // rw
			uint8_t sweep_number : 3; // 0-7
			op      sweep_mode   : 1; 
			uint8_t sweep_time   : 3; // x / 128 Hz
			uint8_t _1           : 1; 
		};
		uint8_t NR10;
	};

	union {
		struct { // rw
			uint8_t sound_length : 6; // len = (64 - t)*(1/256) sec
			uint8_t wave_duty    : 2; // 12.5%, 25%, 50%, 75%
		}; 
		uint8_t NR11;
	};

	union {
		struct { // rw
			uint8_t   env_sweep     : 3; // 0 = stop
			direction env_direction : 1;
			uint8_t   env_volume    : 4; // n: 0 = no sound
		}; 
		uint8_t NR12;
	};

	union {
		uint8_t freq_lo; // wo
		uint8_t NR13;
	};

	union {
		struct {
			uint8_t freq_hi     : 3; // wo
			uint8_t _2          : 3;
			uint8_t no_loop     : 1; // rw
			uint8_t init        : 1; // wo
		};
		uint8_t NR14;
	};
};

struct Sound::CH2 {

	union {
		struct {
			uint8_t sound_length : 6; // wo, len = (64 - t)*(1/256) sec
			uint8_t wave_duty    : 2; // rw, 12.5%, 25%, 50%, 75%
		}; 
		uint8_t NR21;		
	};

	union {
		struct { // rw
			uint8_t   env_sweep     : 3; // 0 = stop
			direction env_direction : 1;
			uint8_t   env_volume    : 4; // n: 0 = no sound
		}; 
		uint8_t NR22;
	};

	union {
		uint8_t freq_lo; // wo
		uint8_t NR23;
	};

	union {
		struct {
			uint8_t freq_hi     : 3; // wo
			uint8_t _           : 3;
			uint8_t no_loop     : 1; // rw
			uint8_t init        : 1; // wo
		}; 
		uint8_t NR24;
	};
};

struct Sound::CH3 {
	union {
		struct {
			uint8_t _1       : 7;
			uint8_t sound_on : 1; // rw
		}; 
		uint8_t NR30;
	};

	union {
		uint8_t sound_length;
		uint8_t NR31;
	};

	union {
		struct {
			uint8_t _2     : 4;
			uint8_t volume : 2; // rw
			uint8_t _3     : 4;
			// 0=0% 1=100% 2=50% 3=25%
		}; 
		uint8_t NR32;
	};

	union { // wo
		uint8_t freq_lo; 
		uint8_t NR33;
	};

	union {
		struct {
			uint8_t freq_hi     : 3; // wo
			uint8_t _4          : 3;
			uint8_t no_loop     : 1; // rw
			uint8_t init        : 1; // wo
		}; 
		uint8_t NR34;
	};

	// Wave Pattern RAM: FF30 - FF3F
};

struct Sound::CH4 {
	union {
		struct {
			uint8_t sound_length : 6;
			uint8_t _1     : 2;
		}; 
		uint8_t NR41;
	};

	union {
		struct {
			uint8_t env_sweep       : 3; // 0 = stop
			direction env_direction : 1;
			uint8_t env_volume      : 4; // n: 0 = no sound
		}; 
		uint8_t NR42;
	};

	union {
		struct {
			uint8_t freq_div : 3; // r
			uint8_t counter_step : 1; // 0=15 bits, 1=7 bits
			uint8_t shift_clk_freq : 4; // s
			// freq = 524288 / r / 2^(s+1) Hz  (assume r=0.5 when 0)
		}; 
		uint8_t NR43;
	};

	union {
		struct {
			uint8_t _2          : 6;
			uint8_t no_loop     : 1; // rw
			uint8_t init        : 1; // wo
		}; 
		uint8_t NR44;
	};
};

struct Sound::CTRL {
	union {
		struct { // rw
			uint8_t SO1_vol : 3;
			uint8_t SO1_vin : 1;
			uint8_t SO2_vol : 3;
			uint8_t SO2_vin : 1;
		};
		uint8_t NR50;
	};

	union {
		struct { // rw
			uint8_t CH1_SO1 : 1;
			uint8_t CH2_SO1 : 1;
			uint8_t CH3_SO1 : 1;
			uint8_t CH4_SO1 : 1;
			uint8_t CH1_SO2 : 1;
			uint8_t CH2_SO2 : 1;
			uint8_t CH3_SO2 : 1;
			uint8_t CH4_SO2 : 1;
		};
		uint8_t NR51;
	};

	union {
		struct {
			uint8_t CH1_on   : 1; // ro
			uint8_t CH2_on   : 1; // ro
			uint8_t CH3_on   : 1; // ro
			uint8_t CH4_on   : 1; // ro
			uint8_t _        : 3;
			uint8_t sound_on : 1; // rw
		};
		uint8_t NR52;
	};
};

Sound::Sound() : clock(0), lsample(0), rsample(0) {
	Channel1 = new CH1();
	Channel2 = new CH2();
	Channel3 = new CH3();
	Channel4 = new CH4();
	Control  = new CTRL();

	reg_pointers = {
		{0xFF10, &(Channel1->NR10)},
		{0xFF11, &(Channel1->NR11)},
		{0xFF12, &(Channel1->NR12)},
		{0xFF13, &(Channel1->NR13)},
		{0xFF14, &(Channel1->NR14)},

		{0xFF16, &(Channel2->NR21)},
		{0xFF17, &(Channel2->NR22)},
		{0xFF18, &(Channel2->NR23)},
		{0xFF19, &(Channel2->NR24)},

		{0xFF1A, &(Channel3->NR30)},
		{0xFF1B, &(Channel3->NR31)},
		{0xFF1C, &(Channel3->NR32)},
		{0xFF1D, &(Channel3->NR33)},
		{0xFF1E, &(Channel3->NR34)},

		{0xFF20, &(Channel4->NR41)},
		{0xFF21, &(Channel4->NR42)},
		{0xFF22, &(Channel4->NR43)},
		{0xFF23, &(Channel4->NR44)},

		{0xFF24, &(Control->NR50)},
		{0xFF25, &(Control->NR51)},
		{0xFF26, &(Control->NR52)}
	};
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

void Sound::update(unsigned tclk) {
	clock += tclk;
/*
	float test_hz = 220;
	static unsigned test_ctr = 0;
	unsigned wave_samples = SAMPLE_RATE / test_hz;
*/
	if (clock >= TCLK_HZ / SAMPLE_RATE) {
		clock -= TCLK_HZ / SAMPLE_RATE;
		sample_ready = true;

/*
		uint8_t val;
		if ((++test_ctr) % wave_samples > wave_samples/2)
			val = 127;
		else
			val = 0; 

		lsample = val;
		rsample = val;
*/

		unsigned new_rsample = 0;
		unsigned new_lsample = 0;

		if (Control->sound_on) {
			uint8_t ch1_sample = updateCh1();
			uint8_t ch2_sample = updateCh2();
			uint8_t ch4_sample = updateCh4();

			if (Control->CH1_SO1) new_lsample += ch1_sample;
			if (Control->CH1_SO2) new_rsample += ch1_sample;

			if (Control->CH2_SO1) new_lsample += ch2_sample;
			if (Control->CH2_SO2) new_rsample += ch2_sample;

			if (Control->CH4_SO1) new_lsample += ch4_sample;
			if (Control->CH4_SO2) new_rsample += ch4_sample;

			new_lsample = new_lsample > 255 ? 255 : new_lsample;
			new_rsample = new_rsample > 255 ? 255 : new_rsample;
		}

		// TODO: volume control
		lsample = Control->SO1_vol ? uint8_t(new_lsample) : 0;
		rsample = Control->SO2_vol ? uint8_t(new_rsample) : 0;
	}
	
}

bool Sound::hasNewSample() {
	return sample_ready;
}

void Sound::getSamples(uint8_t * left, uint8_t * right) {
	*left = lsample;
	*right = rsample;
	sample_ready = false;
}

uint8_t Sound::updateCh1() {
	static bool active = false;
	static unsigned ctr = 0;
	static float length = 0;
	static float env_step = 0;
	static float env_ctr = 0;
	static uint8_t vol = 0;

	uint8_t sample = 0;

	if (Channel1->init) {
		active = true;
		Channel1->init = false;
		if (!Channel1->no_loop)
			length = float(64 - Channel1->sound_length) / 256;
		ctr = 0;

		env_step = float(Channel1->env_sweep) / 64.0f;
		env_ctr  = 0;
		vol = Channel1->env_volume << 4;

		Control->CH1_on = true;
		printf("[ch1] init\n");
	}

	if (active) {
		unsigned gb_freq = unsigned(Channel1->freq_hi) << 8;
		gb_freq |= Channel1->freq_lo;
		unsigned hz = 131072 / (2048 - gb_freq);

		unsigned wavelen = unsigned(SAMPLE_RATE / hz);
		const static float duty_map[4] { 1.0f/8, 1.0f/4, 1.0f/2, 3.0f/4 };
		bool low = ((++ctr) % wavelen) > (duty_map[Channel1->wave_duty] * wavelen);

		sample = low ? 0 : vol;

		if (Channel1->no_loop) {
			length -= 1.0f / float(SAMPLE_RATE);
			if (length <= 0) {
				active = false;
				Control->CH1_on = false;
				printf("[ch1] stop\n");
			}
		}

		if (Channel1->env_sweep != 0) {
			env_ctr += 1.0f / float(SAMPLE_RATE);
			if (env_ctr > env_step) {
				env_ctr -= env_step;
				
				if (Channel1->env_direction == Increase) {
					printf("[ch1] vol=%02X env+\n", vol);
					if (vol != 0xF0) vol += 0x10;
				} else {
					printf("[ch1] vol=%02X env-\n", vol);
					if (vol & 0xF0) vol -= 0x10;
				}
			}
		}
	}

	return sample;
}

uint8_t Sound::updateCh2() {
	static bool active = false;
	static unsigned ctr = 0;
	static float length = 0;
	static float env_step = 0;
	static float env_ctr = 0;
	static uint8_t vol = 0;

	uint8_t sample = 0;

	if (Channel2->init) {
		active = true;
		Channel2->init = false;
		if (!Channel2->no_loop)
			length = float(64 - Channel2->sound_length) / 256;
		ctr = 0;

		env_step = float(Channel2->env_sweep) / 64.0f;
		env_ctr  = 0;
		vol = Channel2->env_volume << 4;

		Control->CH2_on = true;
		printf("[ch2] init\n");
	}

	if (active) {
		unsigned gb_freq = unsigned(Channel2->freq_hi) << 8;
		gb_freq |= Channel2->freq_lo;
		unsigned hz = 131072 / (2048 - gb_freq);

		unsigned wavelen = unsigned(SAMPLE_RATE / hz);
		const static float duty_map[4] { 1.0f/8, 1.0f/4, 1.0f/2, 3.0f/4 };
		bool low = ((++ctr) % wavelen) > (duty_map[Channel2->wave_duty] * wavelen);

		sample = low ? 0 : vol;

		if (Channel2->no_loop) {			
			length -= 1.0f / float(SAMPLE_RATE);
			if (length <= 0) {
				active = false;
				Control->CH2_on = false;
				printf("[ch2] stop\n");
			}
		}

		if (Channel2->env_sweep != 0) {
			env_ctr += 1.0f / float(SAMPLE_RATE);
			if (env_ctr > env_step) {
				env_ctr -= env_step;
				if (Channel2->env_direction == Increase) {
					printf("[ch2] vol=%02X env+\n", vol);
					if (vol != 0xF0) vol += 0x10;
				} else {
					printf("[ch2] vol=%02X env-\n", vol);
					if (vol & 0xF0) vol -= 0x10;
				}
			}
		}
	}

	return sample;
}

uint8_t Sound::updateCh3() {
	return 0;	
}

uint8_t Sound::updateCh4() {
	static bool active = false;
	static unsigned ctr = 0;
	static float length = 0;
	static float env_step = 0;
	static float env_ctr = 0;
	static uint8_t vol = 0;
	static bool low = 0;

	uint8_t sample = 0;

	if (Channel4->init) {
		active = true;
		Channel4->init = false;
		if (!Channel4->no_loop)
			length = float(64 - Channel4->sound_length) / 256;
		ctr = 0;

		env_step = float(Channel4->env_sweep) / 64.0f;
		env_ctr  = 0;
		vol = Channel4->env_volume << 4;

		Control->CH4_on = true;
		printf("[ch4] init\n");
	}

	if (active) {
		/*
	union {
		struct {
			uint8_t freq_div : 3; // r
			uint8_t counter_step : 1; // 0=15 bits, 1=7 bits
			uint8_t shift_clk_freq : 4; // s
			// freq = 524288 / r / 2^(s+1) Hz  (assume r=0.5 when 0)
		}; 
		uint8_t NR43;
	};

		*/
		float r = Channel4->freq_div;
		if (r == 0) r = 0.5f;
		unsigned s = Channel4->shift_clk_freq;
		float hz = 524288.0f / r / (2 << (s + 1));
		unsigned wavelen = unsigned(SAMPLE_RATE / hz);
		if (wavelen == 0) wavelen = 1;
		// TODO: Channel5->counter_step

		if (((++ctr) % wavelen) > (wavelen / 2)) low = rand() % 2;
		sample = low ? 0 : vol;

		if (Channel4->no_loop) {
			length -= 1.0f / float(SAMPLE_RATE);
			if (length <= 0) {
				active = false;
				Control->CH4_on = false;
				printf("[ch4] stop\n");
			}
		}

		if (Channel4->env_sweep != 0) {
			env_ctr += 1.0f / float(SAMPLE_RATE);
			if (env_ctr > env_step) {
				env_ctr -= env_step;
				if (Channel4->env_direction == Increase) {
					printf("[ch4] vol=%02X env+\n", vol);
					if (vol != 0xF0) vol += 0x10;
				} else {
					printf("[ch4] vol=%02X env-\n", vol);
					if (vol & 0xF0) vol -= 0x10;
				}
			}
		}
	}

	return sample;
}