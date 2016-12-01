#pragma once

#define BIT_0 0x01
#define BIT_1 0x02
#define BIT_2 0x04
#define BIT_3 0x08
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80

class Registers {
public:
	union {
		uint16_t AF;
		struct {
			union {
				uint8_t F;
				struct {
					uint8_t _ : 4;
					bool FLAG_C : 1;
					bool FLAG_H : 1;
					bool FLAG_N : 1;
					bool FLAG_Z : 1;
				};
			};
			uint8_t A;
		};
	};

	union {
		uint16_t BC;
		struct {
			uint8_t C;
			uint8_t B;
		};
	};

	union {
		uint16_t DE;
		struct {
			uint8_t E;
			uint8_t D;
		};
	};

	union {
		uint16_t HL;
		struct {
			uint8_t L;
			uint8_t H;
		};
	};

	uint16_t SP;
	uint16_t PC;
	uint64_t TCLK;
	bool IME;

	bool HALT;
};
