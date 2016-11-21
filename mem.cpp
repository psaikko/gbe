#include <cstring>
#include <cstdio>
#include <cassert>
#include <cstdlib>

#include "mem.h"

void Memory::loadROMBank(int new_bank) {
	// Load 32K ROM bank
	// printf("Selecting ROM bank %d\n", new_bank);
	memcpy(ROM1, &ROM_BANKS[0x4000 * new_bank], 0x4000);
	rom_bank = new_bank;
}

void Memory::loadRAMBank(int new_bank) {
	// Store current RAM bank
	// printf("Selecting RAM bank %d\n", new_bank);
	memcpy(&RAM_BANKS[0x2000 * ram_bank], extRAM, 0x2000);
	// Load new 8K RAM bank
	memcpy(extRAM, &RAM_BANKS[0x2000 * new_bank], 0x2000);
	ram_bank = new_bank;
}

uint8_t* Memory::getReadPtr(uint16_t addr) {
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

uint8_t* Memory::getWritePtr(uint16_t addr) {
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

uint8_t Memory::readByte(uint16_t addr) {
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

uint16_t Memory::readWord(uint16_t addr) {
	if (break_addr == addr) at_breakpoint = true;
	uint8_t *ptr = getReadPtr(addr);
	if (ptr != nullptr)
		return *reinterpret_cast<uint16_t*>(ptr); 
	else {
		fprintf(stdout, "[Warning] Attempting read from address 0x%04X\n", addr);
		return 0;
	}
}

void Memory::writeByte(uint16_t addr, uint8_t val) {
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

void Memory::writeWord(uint16_t addr, uint16_t val) {
	if (break_addr == addr) at_breakpoint = true;
	uint8_t *ptr = getWritePtr(addr);

	if (ptr == nullptr) {
		fprintf(stdout, "[Warning] Attempting write to readonly address 0x%04X\n", addr);
		return;
	}
	uint16_t *wptr = reinterpret_cast<uint16_t*>(ptr); 
	*wptr = val;
}