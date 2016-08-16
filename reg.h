#pragma once

#define FLAG_C 0x10
#define FLAG_H 0x20
#define FLAG_N 0x40
#define FLAG_Z 0x80

#define BIT_0 0x01
#define BIT_1 0x02
#define BIT_2 0x04
#define BIT_3 0x08
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80

typedef struct {
	struct {
		union {
			uint16_t AF;
			struct {
				uint8_t F;
				uint8_t A;
			};
		};
	};

	struct {
		union {
			uint16_t BC;
			struct {
				uint8_t C;
				uint8_t B;
			};
		};
	};

	struct {
		union {
			uint16_t DE;
			struct {
				uint8_t E;
				uint8_t D;
			};
		};
	};

	struct {
		union {
			uint16_t HL;
			struct {
				uint8_t L;
				uint8_t H;
			};
		};
	};

	uint16_t SP;
	uint16_t PC;
	uint64_t TCLK;
	bool IME;

	bool HALT;
} registers;

registers REG;

void set_flag(uint8_t flag) {
	REG.F |= flag;
}

void set_flag_cond(uint8_t flag, bool on) {
	if (on) REG.F |= flag;
	else    REG.F &= ~flag;
}

void unset_flag(uint8_t flag) {
	REG.F &= ~flag;
}

bool get_flag(uint8_t flag) {
	return REG.F & flag;
}