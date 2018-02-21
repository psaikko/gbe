#include "sound.h"
#include <cassert>
#include <limits>

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
			uint8_t disable_loop     : 1; // rw
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
			uint8_t disable_loop     : 1; // rw
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
			uint8_t disable_loop     : 1; // rw
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
			uint8_t disable_loop     : 1; // rw
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

struct Sound::LengthCounter {
	LengthCounter(unsigned max) : max_length(max), length(0) {}

	void start(unsigned sound_len) {
		assert(sound_len <= max_length);
		length = (max_length - sound_len) * TCLK_HZ / 256;
	}

	bool tick(unsigned clk) {
		length -= clk;
		return length <= 0;
	}

	private:
		unsigned max_length;
		int length;

};

Sound::Sound() : samples(0), clock(0), lsample(0), rsample(0) {
	Channel1 = new CH1();
	Channel2 = new CH2();
	Channel3 = new CH3();
	Channel4 = new CH4();
	Control  = new CTRL();

	Ch1_Length = new LengthCounter(64);
	Ch2_Length = new LengthCounter(64);
	Ch3_Length = new LengthCounter(256);
	Ch4_Length = new LengthCounter(64);

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

	mute_ch1 = false;
	mute_ch2 = false;
	mute_ch3 = false;
	mute_ch4 = false;

	sample_t max_sample = std::numeric_limits<sample_t>::max() / 4;
	sample_t min_sample = std::numeric_limits<sample_t>::min() / 4;

	for (unsigned i = 0; i < 16; ++i) {
		sample_map[i] = max_sample - ((max_sample - min_sample) * i) / 15;
	}

	for (unsigned i = 0; i < 33; ++i) {
		square_map[i] = max_sample - ((max_sample - min_sample) * i) / 32;
	}
}

void Sound::writeByte(uint16_t addr, uint8_t val) {
	// mask out readonly bits of FF26
	if (addr == 0xFF26) val &= 0x80;
	if (addr == 0xFF15 || addr == 0xFF1F || (addr >= 0xFF27 && addr <= 0xFF2F)) {
		printf("[snd] write %02X to unused register %04X\n", val, addr);
	} else if (addr >= 0xFF30 && addr <= 0xFF3F) {
		wave_pattern_ram[addr & 0x000F] = val;
	} else {
		*reg_pointers[addr] = val;
	}
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
	if (addr == 0xFF15 || addr == 0xFF1F || (addr >= 0xFF27 && addr <= 0xFF2F)) {
		printf("[snd] read from unused register %04X\n", addr);
		return 0;
	} else if (addr >= 0xFF30 && addr <= 0xFF3F) {
		return wave_pattern_ram[addr & 0x000F];
	} else {
		//printf("[snd] read from register %04X\n", addr);
		uint8_t val = *reg_pointers[addr];
		return val & mask;
	}
}

void Sound::update(unsigned tclk) {
	clock += tclk * SAMPLE_RATE;

	sample_t ch3_sample = updateCh3(tclk);
	sample_t ch2_sample = updateCh2(tclk);
	sample_t ch1_sample = updateCh1(tclk);
	sample_t ch4_sample = updateCh4(tclk);

	// generate a sample every TCLK_HS / SAMPLE_RATE clocks
	if (clock >= TCLK_HZ) {
		clock -= TCLK_HZ;
		sample_ready = true;

		samples++;

		rsample = 0;
		lsample = 0;

		if (Control->sound_on) {

			if (!mute_ch1) {
				if (Control->CH1_SO1) lsample += ch1_sample;
				if (Control->CH1_SO2) rsample += ch1_sample;
			}

			if (!mute_ch2) {
				if (Control->CH2_SO1) lsample += ch2_sample;
				if (Control->CH2_SO2) rsample += ch2_sample;
			}

			if (!mute_ch3) {
				if (Control->CH3_SO1) lsample += ch3_sample;
				if (Control->CH3_SO2) rsample += ch3_sample;
			}

			if (!mute_ch4) {
				if (Control->CH4_SO1) lsample += ch4_sample;
				if (Control->CH4_SO2) rsample += ch4_sample;
			}
		}

		// TODO: volume control
		lsample = Control->SO1_vol ? sample_t(lsample) : 0;
		rsample = Control->SO2_vol ? sample_t(rsample) : 0;

		//printf("[snd] %d\n", rsample);
	}
}

bool Sound::hasNewSample() {
	return sample_ready;
}

void Sound::getSamples(sample_t * left, sample_t * right) {
	*left = lsample;
	*right = rsample;
	sample_ready = false;
}

sample_t Sound::updateCh1(unsigned tclock) {

	static unsigned freq_clock = 0;

	freq_clock += tclock;

	static bool active = false;

	static unsigned ctr = 0;
	static int length = 0;
	static unsigned env_step = 0;
	static unsigned env_ctr = 0;
	static unsigned sweep_ctr = 0;
	static unsigned sweep_step = 0;
	static unsigned sweep_freq = 0;

	static uint8_t vol = 0;

	static sample_t sample = 0;

	if (Channel1->init) {
		freq_clock = 0;
		active = true;
		Channel1->init = 0;

		if (Channel1->disable_loop)
			length = (64 - Channel1->sound_length) * TCLK_HZ / 256;
		ctr = 0;

		// hz = env_step / 64
		env_step = Channel1->env_sweep * TCLK_HZ / 64;
		env_ctr  = 0;

		// hz = env_step / 128
		sweep_step = Channel1->sweep_time * TCLK_HZ / 128;
		sweep_ctr = 0;

		vol = Channel1->env_volume;

		sweep_freq = unsigned(Channel1->freq_hi) << 8;
		sweep_freq |= Channel1->freq_lo;

		Control->CH1_on = 1;
		//printf("[ch1] init\n");
	}

	if (active) {
		// waveform control
		unsigned gb_freq = 2048 - (unsigned(Channel1->freq_lo) + (unsigned(Channel1->freq_hi) << 8));
		if (freq_clock >= gb_freq) {
			freq_clock -= gb_freq;

			// gb_freq = frequency in tclocks / 32
			// i.e. 32 ticks per wavelength

			const static unsigned duty_map[4] { 4, 8, 16, 24 };

			bool low = ((++ctr) % 32) > duty_map[Channel1->wave_duty];

			sample = low ? square_map[16 + vol] : square_map[16 - vol];

			if (Channel1->disable_loop) {
				length -= tclock;
				if (length <= 0) {
					active = false;
					Control->CH1_on = 0;
					//printf("[ch1] stop\n");
				}
			}
		}

		// frequency sweep control
		if (sweep_step != 0) {
			sweep_ctr += tclock;
			if (sweep_ctr >= sweep_step) {
				sweep_ctr -= sweep_step;
				if (Channel1->sweep_mode == CH1::op::Addition) {
					//printf("[ch1] sweep +\n");
					sweep_freq += sweep_freq >> Channel1->sweep_number;
				} else {
					//printf("[ch1] sweep -\n");
					sweep_freq -= sweep_freq >> Channel1->sweep_number;
				}
				if (sweep_freq & 0xF800) {
					active = false;
					Control->CH1_on = 0;
					//printf("[ch1] sweep stop\n");
				} else {
					Channel1->freq_hi = sweep_freq >> 8;
					Channel1->freq_lo = sweep_freq & 0xFF;
				}
			}
		}

		// volume envelope control
		if (env_step != 0) {
			env_ctr += tclock;

			if (env_ctr >= env_step) {
				env_ctr -= env_step;

				if (Channel1->env_direction == Increase) {
					//printf("[ch1] vol=%02X env+\n", vol);
					if (vol != 0x0F) vol ++;
				} else {
					//printf("[ch1] vol=%02X env-\n", vol);
					if (vol & 0x0F) vol --;
				}
			}
		}
	}

	return sample;
}

sample_t Sound::updateCh2(unsigned tclock) {

	static unsigned freq_clock = 0;

	freq_clock += tclock;

	static bool active = false;

	static unsigned ctr = 0;
	static unsigned env_step = 0;
	static unsigned env_ctr = 0;

	static uint8_t vol = 0;

	static sample_t sample = 0;

	if (Channel2->init) {
		freq_clock = 0;
		active = true;
		Channel2->init = 0;

		if (Channel2->disable_loop)
			Ch2_Length->start(Channel2->sound_length);

		ctr = 0;

		// hz = env_step / 64
		env_step = Channel2->env_sweep * TCLK_HZ / 64.0f;
		env_ctr  = 0;

		vol = Channel2->env_volume;

		Control->CH2_on = 1;
		//printf("[ch2] init\n");
	}

	if (active) {
		// waveform control
		unsigned gb_freq = 2048 - (unsigned(Channel2->freq_lo) + (unsigned(Channel2->freq_hi) << 8));
		if (freq_clock >= gb_freq) {
			freq_clock -= gb_freq;

			// gb_freq = frequency in tclocks / 32
			// i.e. 32 ticks per wavelength

			const static float duty_map[4] { 4, 8, 16, 24 };

			bool low = ((++ctr) % 32) > duty_map[Channel2->wave_duty];

			sample = low ? square_map[16 + vol] : square_map[16 - vol];

			if (Channel2->disable_loop) {
				if (Ch2_Length->tick(tclock)) {
					active = false;
					Control->CH2_on = 0;
					//printf("[ch2] stop\n");
				}
			}
		}

		// volume envelope control
		if (env_step != 0) {
			env_ctr += tclock;

			if (env_ctr >= env_step) {
				env_ctr -= env_step;

				if (Channel2->env_direction == Increase) {
					//printf("[ch2] vol=%02X env+\n", vol);
					if (vol != 0xF) vol ++;
				} else {
					//printf("[ch2] vol=%02X env-\n", vol);
					if (vol & 0xF) vol --;
				}
			}
		}
	}

	return sample;
}

sample_t Sound::updateCh3(unsigned tclock) {

	static unsigned freq_clock = 0;
	static uint8_t index = 0;

	freq_clock += tclock;

	static bool active = false;
	static uint8_t vol = 0;

	if (Channel3->init) {
		freq_clock = 0;
		active = true;
		Channel3->init = 0;
		if (Channel3->disable_loop)
			Ch3_Length->start(Channel3->sound_length);
		index = 0;
		Control->CH3_on = 1;
		vol = Channel3->volume;
		//printf("[ch3] init\n");
	}

	static sample_t sample = 0;

	unsigned gb_freq = 2048 - (unsigned(Channel3->freq_lo) + (unsigned(Channel3->freq_hi) << 8));
	gb_freq = gb_freq * TCLK_HZ / 65536; // sample played at freq * 65536 hz

	if (freq_clock >= gb_freq / 32) {
		freq_clock -= gb_freq / 32;

		if (active && Channel3->sound_on) {
			index = (index + 1) % 32;

			// 4-bit samples played high bits first
			uint8_t wave_sample = wave_pattern_ram[(31 - index) >> 1];
			// (index flip changes parity!)
			if (!(index % 2)) wave_sample &= 0x0F; // low 4 bytes
			else 		   wave_sample >>= 4;   // high 4 bytes

			// center waveform on 0
			int wave_sample_signed = int(wave_sample) - 8;

			// apply volume shift
			static const uint8_t volume_shift_map[4] { 4, 0, 1, 2 };
			wave_sample_signed >>= volume_shift_map[vol];

			sample = sample_map[wave_sample_signed + 8];

			//printf("[ch3] %d %d\n", wave_sample << volume_shift_map[vol], wave_sample);
			if (Channel3->disable_loop) {
				if (Ch3_Length->tick(tclock)) {
					active = false;
					Control->CH3_on = 0;
					//printf("[ch3] stop\n");
				}
			}
		}
	}

	return sample;
}

sample_t Sound::updateCh4(unsigned tclock) {

	static unsigned freq_clock = 0;

	freq_clock += tclock;

	static bool active = false;

	static unsigned env_step = 0;
	static unsigned env_ctr = 0;

	static uint16_t counter = 0;

	static uint8_t vol = 0;

	static sample_t sample = 0;

	if (Channel4->init) {
		freq_clock = 0;
		active = true;
		Channel4->init = 0;

		if (Channel4->disable_loop)
			Ch4_Length->start(Channel4->sound_length);

		// hz = env_step / 64
		env_step = Channel4->env_sweep * TCLK_HZ / 64;
		env_ctr  = 0;
		/*
		if (env_step != 0) {
			printf("[ch4] env start %d\n", Channel4->env_sweep);
		}
		*/
		vol = Channel4->env_volume;

		Control->CH4_on = 1;
		//printf("[ch4] init\n");

		// initialize counter with 15 1-bits

		//printf("[ch4] %02X %02X %02X %02X\n", Channel4->NR41,
		//	Channel4->NR42, Channel4->NR43, Channel4->NR44);
		counter = (1 << 15) - 1;
	}

	if (active) {
		// compute frequency
   		const static uint8_t divisor_lookup[8] { 8, 16, 32, 48, 64, 80, 96, 112 };

   		unsigned counter_clk = (TCLK_HZ / 8) / divisor_lookup[Channel4->freq_div];
		unsigned gb_freq = counter_clk >> (Channel4->shift_clk_freq + 1);

		gb_freq >>= 4; // TODO: magic constant --- fixme

		if (freq_clock >= gb_freq) {
			freq_clock -= gb_freq;

			// feedback bit is bit0 xor bit1
			bool feedback = bool(counter & 2) ^ bool(counter & 1);
			// shift right
			counter >>= 1;

			if (feedback) {
				// feedback xor to bit 14
				counter &= ~(1 << 14);
				counter |= (1 << 14);

				// also feedback to bit 6?
				if (Channel4->counter_step) {
					counter &= ~(1 << 6);
					counter |= (1 << 6);
				}
			}

			// output is inverted 0-bit of counter
			bool low = ~counter & 1;

			sample = low ? square_map[16 - vol] : square_map[16 + vol];

			//printf("%02X %02X %02X %02X\n", Channel4->NR41,
			//	Channel4->NR42, Channel4->NR43, Channel4->NR44);
		}

		if (Channel4->disable_loop) {
			if (Ch4_Length->tick(tclock)) {
				active = false;
				Control->CH4_on = 0;
				//printf("[ch4] stop\n");
			}
		}

		// volume sweep control
		if (env_step != 0) {
			env_ctr += tclock;
			if (env_ctr >= env_step) {
				env_ctr -= env_step;
				//printf("[ch4] asdf %02X %02X %02X %02X\n", Channel4->NR41,
				//	Channel4->NR42, Channel4->NR43, Channel4->NR44);
				if (Channel4->env_direction == Increase) {
					//printf("[ch4] vol=%02X env+\n", vol);
					if (vol != 0xF) vol ++;
				} else {
					//printf("[ch4] vol=%02X env-\n", vol);
					if (vol & 0xF) vol --;
				}
			}
		}
	}

	return sample;
}