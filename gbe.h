#pragma once
#include <inttypes.h>

typedef void (*instr_fn)(void);

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
} registers;

uint8_t FLAG_C = 0x10;
uint8_t FLAG_H = 0x20;
uint8_t FLAG_N = 0x40;
uint8_t FLAG_Z = 0x80;

typedef struct {
	uint8_t ROM0[16384];  // [0000-3FFF] 
	uint8_t ROM1[16384];  // [4000-7FFF]
	uint8_t grRAM[8192];  // [8000-9FFF] 
	uint8_t extRAM[8192]; // [A000-BFFF]
	uint8_t RAM[8192];    // [C000-DFFF]
	uint8_t _RAM[7680];   // [E000-FDFF]
	uint8_t SPR[255];     // [FE00-FE9F] // 160 bytes?
	uint8_t IO[128];      // [FF00-FF7F] 
	uint8_t ZERO[128];    // [FF80-FFFF]
} memory;

typedef struct {
	char name[16];
	uint8_t arity;
	instr_fn fn;
} instruction;