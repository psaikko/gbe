#pragma once

#include "reg.h"
#include <cstring>

#define FLAG_GPU_BG     0x01
#define FLAG_GPU_SPR    0x02
#define FLAG_GPU_SPR_SZ 0x04
#define FLAG_GPU_BG_TM  0x08
#define FLAG_GPU_BG_WIN_TS  0x10
#define FLAG_GPU_WIN    0x20
#define FLAG_GPU_WIN_TM 0x40
#define FLAG_GPU_DISP   0x80

#define PLT_COLOR0 0x03
#define PLT_COLOR1 0x0C
#define PLT_COLOR2 0x30
#define PLT_COLOR3 0xC0

uint8_t key_state;
#define KEY_RIGHT  0x01
#define KEY_LEFT   0x02
#define KEY_UP     0x04
#define KEY_DOWN   0x08
#define KEY_A      0x10
#define KEY_B      0x20
#define KEY_START  0x40
#define KEY_SELECT 0x80

typedef struct {
	uint8_t y;
	uint8_t x;
	uint8_t tile_id;
	struct {
		union {
			uint8_t flags;
			struct {
				uint8_t _ : 4;
				uint8_t palette : 1;
				uint8_t xflip : 1;
				uint8_t yflip : 1;
				uint8_t priority : 1;
			};
		};
	};
} oam_entry;

typedef struct {

	char ROM_BANKS[8388608]; // Max ROM size 8 MB
	char RAM_BANKS[131072];  // Max RAM size 128 KB
	int rom_size;

	uint8_t RAW[65536]; // TODO
	uint8_t *ROM0   = &RAW[0x0000];

	int rom_bank;
	uint8_t *ROM1   = &RAW[0x4000];
	uint8_t *grRAM  = &RAW[0x8000];
	int ram_bank;
	uint8_t *extRAM = &RAW[0xA000];
	uint8_t *RAM    = &RAW[0xC000];
	uint8_t *_RAM   = &RAW[0xE000];
	//uint8_t *OAM    = &RAW[0xFE00]; // sprite attribute table
	oam_entry *OAM  = (oam_entry*)(&RAW[0xFE00]);
	uint8_t *IO     = &RAW[0xFF00];
	uint8_t *ZERO   = &RAW[0xFF80];

	uint8_t BIOS[256];	 

	uint8_t *IE       = &RAW[0xFFFF];
	uint8_t *DIV      = &RAW[0xFF04];
	uint8_t *TIMA     = &RAW[0xFF05];
	uint8_t *TMA      = &RAW[0xFF06];
	uint8_t *TAC      = &RAW[0xFF07];
	uint8_t *IF       = &RAW[0xFF0F];
	uint8_t *LCD_CTRL = &RAW[0xFF40];
	uint8_t *LCD_STAT = &RAW[0xFF41];
	uint8_t *SCRL_Y   = &RAW[0xFF42];
	uint8_t *SCRL_X   = &RAW[0xFF43];
	uint8_t *SCAN_LN  = &RAW[0xFF44]; // TODO: readonly
	uint8_t *LN_CMP   = &RAW[0xFF45];
	uint8_t *OAM_DMA  = &RAW[0xFF46]; // TODO: writeonly
	uint8_t *BG_PLT   = &RAW[0xFF47]; // TODO: writeonly
	uint8_t *OBJ0_PLT = &RAW[0xFF48]; // TODO: writeonly
	uint8_t *OBJ1_PLT = &RAW[0xFF49]; // TODO: writeonly
	uint8_t *WIN_Y    = &RAW[0xFF4A];
	uint8_t *WIN_X    = &RAW[0xFF4B];
	uint8_t *BIOS_OFF = &RAW[0xFF50];

	uint8_t *TILESET1 = &RAW[0x8000];
	uint8_t *TILESET0 = &RAW[0x8800];
	uint8_t *TILEMAP0 = &RAW[0x9800];
	uint8_t *TILEMAP1 = &RAW[0x9C00];

	uint16_t break_addr = 0;
	bool at_breakpoint = false;

	void loadROMBank(int new_bank) {
		// Load 32K ROM bank
		// printf("Selecting ROM bank %d\n", new_bank);
		memcpy(ROM1, &ROM_BANKS[0x4000 * new_bank], 0x4000);
		rom_bank = new_bank;
	}

	void loadRAMBank(int new_bank) {
		// Store current RAM bank
		// printf("Selecting RAM bank %d\n", new_bank);
		memcpy(&RAM_BANKS[0x2000 * ram_bank], extRAM, 0x2000);
		// Load new 8K RAM bank
		memcpy(extRAM, &RAM_BANKS[0x2000 * new_bank], 0x2000);
		ram_bank = new_bank;
	}

	enum mbc_type { NONE, MBC1, MBC2, MBC3, MBC5 };
	mbc_type bank_controller;

	enum controller_mode { ROM_banking, RAM_banking }; // MBC1 mode switch
	controller_mode mbc_mode;


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
/*
		if (addr >= 0xFF04 && addr <= 0xFF07) {
			printf("[timer] 0x%04X write 0x%02X\n", addr, val);
		}
*/
		if (addr == 0xFF00) { // JOYPAD
			//printf("[joypad] write (0x%02X)\n", val);
			*ptr &= 0x0F;
			if (val == 0x10) {
				*ptr |= 0x20; 
				*ptr &= 0xF0;
				// Load A, B, Select, Start bits
				*ptr |= (key_state >> 4);
			} else if (val == 0x20) {
				*ptr |= 0x10;
				*ptr &= 0xF0;
				// Load Right, Left, Up, Down bits
				*ptr |= (key_state & 0x0F);
			} else if (val == 0x30) {
				// TODO: should we reset lower 4 bits here?
				*ptr &= 0xF0;
			} else {
				printf("[Warning] Bad write (0x%02X) to JOYP (0x%04X)\n", val, addr);
			}
			
			return;
		} else if (ptr == OAM_DMA) {
			// TODO: block memory access
			//printf("OAM DMA\n");
			//assert(val <= 0xF1);
			for (uint8_t low = 0x00; low <= 0xF9; ++low) {
				RAW[0xFE00 + low] = RAW[(((uint16_t)val) << 8) + low];
			}
		} else if (addr == 0xFF04) {
			// divider register reset on write
			*ptr = 0;
			return;
		}

		switch (bank_controller) {
			case mbc_type::NONE:
				break;
			case mbc_type::MBC1:
				if (addr <= 0x1FFF) {
					//printf("RAM enable / disable 0x%02X at 0x%04X\n", val, addr);
					return;
				}
				else if (0x2000 <= addr && addr <= 0x3FFF) {
					//printf("ROM bank selection 0x%02X at 0x%04X\n", val, addr);
					val &= 0x1F;
					if (val == 0) val = 1;
					loadROMBank(val);
					return;
				}
				else if (0x4000 <= addr && addr <= 0x5FFF) {
					//printf("RAM bank selection 0x%02X at 0x%04X\n", val, addr);
					val &= 0x03;
					if (mbc_mode == controller_mode::RAM_banking) {
						loadRAMBank(val);
					} else {
						loadROMBank((rom_bank & 0x1F) | (val << 5));
					}
					return;
				}
				else if (0x6000 <= addr && addr <= 0x7FFF) {
					if (val == 1) {
						mbc_mode = controller_mode::RAM_banking;
					} else {
						mbc_mode = controller_mode::ROM_banking;
					}
				}
				break;
			case mbc_type::MBC3:
				if (addr <= 0x1FFF) {
					//printf("RAM enable / disable 0x%02X at 0x%04X\n", val, addr);
					return;
				}
				if (0x2000 <= addr && addr <= 0x3FFF) {
					//printf("ROM bank selection 0x%02X at 0x%04X\n", val, addr);
					val &= 0x7F;
					if (val == 0) val = 1;
					loadROMBank(val);
					return;
				}
				if (0x4000 <= addr && addr <= 0x5FFF) {
					//printf("RAM bank selection 0x%02X at 0x%04X\n", val, addr);
					val &= 0x1F;
					loadRAMBank(val);
					return;
				}
				break;
			default:
				printf("Unimplemented memory bank controller type.");
				exit(1);
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
