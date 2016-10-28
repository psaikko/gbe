#pragma once

#include "reg.h"

#define FLAG_GPU_BG     0x01
#define FLAG_GPU_SPR    0x02
#define FLAG_GPU_SPR_SZ 0x04
#define FLAG_GPU_BG_TM  0x08
#define FLAG_GPU_BG_TS  0x10
#define FLAG_GPU_WIN    0x20
#define FLAG_GPU_WIN_TM 0x40
#define FLAG_GPU_DISP   0x80

#define BG_PLT_COLOR0 0x03
#define BG_PLT_COLOR1 0x0C
#define BG_PLT_COLOR2 0x30
#define BG_PLT_COLOR3 0xC0

typedef struct {
	uint8_t RAW[65536]; // TODO
	uint8_t *ROM0   = &RAW[0x0000];
	uint8_t *ROM1   = &RAW[0x4000];
	uint8_t *grRAM  = &RAW[0x8000];
	uint8_t *extRAM = &RAW[0xA000];
	uint8_t *RAM    = &RAW[0xC000];
	uint8_t *_RAM   = &RAW[0xE000];
	uint8_t *SPR    = &RAW[0xFE00];
	uint8_t *IO     = &RAW[0xFF00];
	uint8_t *ZERO   = &RAW[0xFF80];

	uint8_t BIOS[256];	 

	uint8_t *IE       = &RAW[0xFFFF];
	uint8_t *IF       = &RAW[0xFF0F];
	uint8_t *GPU_CTRL = &RAW[0xFF40];
	uint8_t *SCRL_Y   = &RAW[0xFF42];
	uint8_t *SCRL_X   = &RAW[0xFF43];
	uint8_t *SCAN_LN  = &RAW[0xFF44]; // TODO: readonly
	uint8_t *BG_PLT   = &RAW[0xFF47]; // TODO: writeonly
	uint8_t *BIOS_OFF = &RAW[0xFF50];

	uint8_t *TILESET1 = &RAW[0x8000];
	uint8_t *TILESET0 = &RAW[0x8800];
	uint8_t *TILEMAP0 = &RAW[0x9800];
	uint8_t *TILEMAP1 = &RAW[0x9C00];

	uint16_t break_addr = 0;
	bool at_breakpoint = false;

	uint8_t* getReadPtr(uint16_t addr) {
		// switch by 8192 byte segments
		switch(addr >> 13) {
			case 0:
				if ( ( ! *BIOS_OFF ) && addr < 0x0100) {
					return &BIOS[addr];
				}
			case 1: // ROM0
				return &RAW[addr];
			case 2: // ROM1
			case 3: 
				return &RAW[addr];
			case 4: // grRAM
				return &RAW[addr];
			case 5: // extRAM
				return &RAW[addr];
			case 6: // RAM
				return &RAW[addr];
			case 7: 
			default:
				if (addr < 0xE000) // RAM
					return &RAW[addr];
				else if (addr < 0xFE00) // shadow RAM
					return &RAW[addr & 0xDFFF];
				else {
					switch (addr & 0xFF80) {
						case 0xFE00: // SPR
						case 0xFE80: 
							if (addr < 0xFEA0)
								return &RAW[addr];
							else 
								return nullptr;
						case 0xFF00: // IO
							return &RAW[addr];
						default:
							assert(false);
						case 0xFF80: // ZERO
							return &RAW[addr];
					}
				}
		}
	}

	uint8_t* getWritePtr(uint16_t addr) {
		// switch by 8192 byte segments
		switch(addr >> 13) {
			case 0:
			case 1: // ROM0
			case 2: // ROM1
			case 3: 
				return nullptr;
			case 4: // grRAM
				return &RAW[addr];
			case 5: // extRAM
				return &RAW[addr];
			case 6: // RAM
				return &RAW[addr];
			case 7: 
			default:
				if (addr < 0xE000) // RAM
					return &RAW[addr];
				else if (addr < 0xFE00) // shadow RAM
					return &RAW[addr & 0xDFFF];
				else {
					switch (addr & 0xFF80) {
						case 0xFE00: // SPR
						case 0xFE80: 
							if (addr < 0xFEA0)
								return &RAW[addr];
							else 
								return nullptr;
						case 0xFF00: // IO
							if (addr == 0xFF50 && *BIOS_OFF) {
								return nullptr;
							}
							return &RAW[addr];
						default:
							assert(false);
						case 0xFF80: // ZERO
							return &RAW[addr];
					}
				} 
		}
	}

	uint8_t readByte(uint16_t addr) {
		if (break_addr == addr) at_breakpoint = true;

		uint8_t *ptr = getReadPtr(addr);

		if (addr == 0xFF00) { // JOYPAD
			return ~(*ptr);
		}

		if (ptr != nullptr)
			return *ptr;
		else {
			fprintf(stdout, "[Warning] Attempting read from address 0x%04X\n", addr);
			return 0;
		}
	}

	uint16_t readWord(uint16_t addr) {
		if (break_addr == addr) at_breakpoint = true;
		uint8_t *ptr = getReadPtr(addr);
		if (ptr != nullptr)
			return *reinterpret_cast<uint16_t*>(ptr); 
		else {
			fprintf(stdout, "[Warning] Attempting read from address 0x%04X\n", addr);
			return 0;
		}
	}

	void writeByte(uint16_t addr, uint8_t val) {
		if (break_addr == addr) at_breakpoint = true;

		uint8_t *ptr = getWritePtr(addr);

		if (addr == 0xFF00) { // JOYPAD
			*ptr &= 0x0F;
			if (val == 0x10) {
				*ptr |= 0x20; 
				*ptr &= 0xF0;
				// TODO: Load A, B, Select, Start bits
			} else if (val == 0x20) {
				*ptr |= 0x10;
				*ptr &= 0xF0;
				// TODO: Load Right, Left, Up, Down bits
			} else if (val == 0x30) {
				// TODO: should we reset lower 4 bits here?
				*ptr &= 0xF0;
			} else {
				fprintf(stdout, "[Warning] Bad write (0x%02X) to JOYP (0x%04X)\n", val, addr);
			}
			
			return;
		}

		if (ptr == nullptr) {
			fprintf(stdout, "[Warning] Attempting write to readonly address 0x%04X\n", addr);
			return;
		}
		*ptr = val;
	}

	void writeWord(uint16_t addr, uint16_t val) {
		if (break_addr == addr) at_breakpoint = true;
		uint8_t *ptr = getWritePtr(addr);

		if (ptr == nullptr) {
			fprintf(stdout, "[Warning] Attempting write to readonly address 0x%04X\n", addr);
			return;
		}
		uint16_t *wptr = reinterpret_cast<uint16_t*>(ptr); 
		*wptr = val;
	}
} memory;

memory MEM;

#define ARGBYTE (MEM.readByte(REG.PC + 1))
#define ARGWORD (MEM.readWord(REG.PC + 1))