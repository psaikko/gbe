
class Sound {

	enum direction { Decrease, Increase };

	class CH1 {
		struct NR10 { // rw
			enum op { Addition, Subtraction };

			uint8_t sweep_number : 3; // 0-7
			op      sweep_mode   : 1; 
			uint8_t sweep_time   : 3; // x / 128 Hz
			uint8_t _            : 1; 
		};

		struct NR11 { // rw
			uint8_t sound_length : 6; // len = (64 - t)*(1/256) sec
			uint8_t wave_duty    : 2; // 12.5%, 25%, 50%, 75%
		};

		struct NR12 { // rw
			uint8_t env_sweep       : 3; // 0 = stop
			direction env_direction : 1;
			uint8_t env_volume      : 4; // n: 0 = no sound
		};

		struct NR13 {
			uint8_t freq_lo; // wo
		};

		struct NR14 {
			uint8_t freq_hi     : 3; // wo
			uint8_t _           : 3;
			uint8_t consecutive : 1; // rw
			uint8_t init        : 1; // wo
		};
	};

	class CH2 {

		struct NR21 {
			uint8_t sound_length : 6; // wo, len = (64 - t)*(1/256) sec
			uint8_t wave_duty    : 2; // rw, 12.5%, 25%, 50%, 75%
		};		

		struct NR22 { // rw
			uint8_t env_sweep       : 3; // 0 = stop
			direction env_direction : 1;
			uint8_t env_volume      : 4; // n: 0 = no sound
		};

		struct NR23 {
			uint8_t freq_lo; // wo
		};

		struct NR24 {
			uint8_t freq_hi     : 3; // wo
			uint8_t _           : 3;
			uint8_t consecutive : 1; // rw
			uint8_t init        : 1; // wo
		}
	};

	class CH3 {
		struct NR30 {
			uint8_t _        : 7;
			uint8_t sound_on : 1; // rw
		};

		struct NR31 {
			uint8_t sound_length;
		};

		struct NR32 {
			uint8_t _      : 4;
			uint8_t volume : 2; // rw
			uint8_t __     : 4;
			// 0=0% 1=100% 2=50% 3=25%
		}

		struct NR33 { // wo
			uint8_t freq_lo;
		};

		struct NR24 {
			uint8_t freq_hi     : 3; // wo
			uint8_t _           : 3;
			uint8_t consecutive : 1; // rw
			uint8_t init        : 1; // wo
		};

		// Wave Pattern RAM: FF30 - FF3F
	};

	class CH4 {
		struct NR41 {
			uint8_t length : 6;
			uint8_t _ : 2;
		};

		struct NR42 {
			uint8_t env_sweep       : 3; // 0 = stop
			direction env_direction : 1;
			uint8_t env_volume      : 4; // n: 0 = no sound
		};

		struct NR43 {
			uint8_t freq_div : 3; // r
			uint8_t counter_step : 1; // 0=15 bits, 1=7 bits
			uint8_t shift_clk_freq : 4; // s
			// freq = 524288 / r / 2^(s+1) Hz  (assume r=0.5 when 0)
		};

		struct NR24 {
			uint8_t _           : 6;
			uint8_t consecutive : 1; // rw
			uint8_t init        : 1; // wo
		};
	};

	class CTRL {
		struct NR50 { // rw
			uint8_t SO1_vol : 3;
			uint8_t SO1_vin : 1;
			uint8_t SO2_vol : 3;
			uint8_t SO2_vin : 1;
		};

		struct NR51 { // rw
			uint8_t CH1_SO1 : 1;
			uint8_t CH2_SO1 : 1;
			uint8_t CH3_SO1 : 1;
			uint8_t CH4_SO1 : 1;
			uint8_t CH1_SO2 : 1;
			uint8_t CH2_SO2 : 1;
			uint8_t CH3_SO2 : 1;
			uint8_t CH4_SO2 : 1;
		};

		struct NR52 {
			uint8_t CH1_on   : 1; // ro
			uint8_t CH2_on   : 1; // ro
			uint8_t CH3_on   : 1; // ro
			uint8_t CH4_on   : 1; // ro
			uint8_t _        : 3;
			uint8_t sound_on : 1; // rw
		};
	};
};

// 1 step = n/64 sec