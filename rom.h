#pragma once

#include "mem.h"
#include <iostream>
#include <string>
#include <cstring>
#include <map>

using namespace std;

void readROMFile(Memory &MEM, const string filename) {

		ifstream romfile(filename, ios::binary);
		romfile.seekg(0, romfile.end);
		MEM.rom_size = romfile.tellg();
		romfile.seekg(0, romfile.beg);
		MEM.ROM_BANKS = (uint8_t*)malloc(MEM.rom_size);
		romfile.read((char*)MEM.ROM_BANKS, MEM.rom_size);
		romfile.close();

		// load lower 16K
		memcpy(MEM.ROM0, MEM.ROM_BANKS, 0x4000);
		// load bank 1 to upper 16K
		MEM.loadROMBank(1);

  	// cart data reference http://www.devrs.com/gb/files/gbspec.txt

  	char rom_name[16];
  	memcpy(rom_name, &MEM.RAW[0x0134], 16);
  	printf("NAME:\t%-16s\n", rom_name);
  	printf("TYPE:\t");
  	if (MEM.RAW[0x0143] == 0x80) printf("GBC ");
  	if (MEM.RAW[0x0146] == 0x00) printf("GB ");
  	if (MEM.RAW[0x0146] == 0x03) printf("SGB ");
  	printf("\n");

  	printf("SIZE:\t%dK\n", MEM.rom_size / 1024);

  	map<uint8_t, string> rom_types;
		rom_types[0x00] = "ROM ONLY";
		rom_types[0x01] = "ROM+MBC1";
		rom_types[0x02] = "ROM+MBC1+RAM";
		rom_types[0x03] = "ROM+MBC1+RAM+BATT";
		rom_types[0x05] = "ROM+MBC2";
		rom_types[0x06] = "ROM+MBC2+BATTERY";
		rom_types[0x08] = "ROM+RAM";
		rom_types[0x09] = "ROM+RAM+BATTERY";
		rom_types[0x0B] = "ROM+MMM01";
		rom_types[0x0C] = "ROM+MMM01+SRAM";
		rom_types[0x0D] = "ROM+MMM01+SRAM+BATT";
		rom_types[0x0F] = "ROM+MBC3+TIMER+BATT";
		rom_types[0x10] = "ROM+MBC3+TIMER+RAM+BATT";
		rom_types[0x11] = "ROM+MBC3";
		rom_types[0x12] = "ROM+MBC3+RAM";
		rom_types[0x13] = "ROM+MBC3+RAM+BATT";
		rom_types[0x19] = "ROM+MBC5";
		rom_types[0x1A] = "ROM+MBC5+RAM";
		rom_types[0x1B] = "ROM+MBC5+RAM+BATT";
		rom_types[0x1C] = "ROM+MBC5+RUMBLE";
		rom_types[0x1D] = "ROM+MBC5+RUMBLE+SRAM";
		rom_types[0x1E] = "ROM+MBC5+RUMBLE+SRAM+BATT";
		rom_types[0x1F] = "Pocket Camera";
		rom_types[0xFD] = "Bandai TAMA5";
		rom_types[0xFE] = "Hudson HuC-3";
		rom_types[0xFF] = "Hudson HuC-1";

		uint8_t type = MEM.RAW[0x0147];
		if (rom_types.count(type)) {
			printf("CART:\t%s\n", rom_types[type].c_str());
		} else {
			printf("Unknown type 0x%02X\n", type);
		}

		switch (type) {
			default:
			case 0x00:
				MEM.bank_controller = Memory::mbc_type::NONE;
				break;
			case 0x01:
			case 0x02:
			case 0x03:
				MEM.bank_controller = Memory::mbc_type::MBC1;
				break;
			case 0x05:
			case 0x06:
				MEM.bank_controller = Memory::mbc_type::MBC2;
				break;			
			case 0x0F:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
				MEM.bank_controller = Memory::mbc_type::MBC3;
				break;
			case 0x19:
			case 0x1A:
			case 0x1B:
			case 0x1C:
			case 0x1D:
			case 0x1E:
				MEM.bank_controller = Memory::mbc_type::MBC5;
				break;
		}	

		map<uint8_t, string> rom_sizes;
		rom_sizes[0x00] = "32KB / 2 banks";
		rom_sizes[0x01] = "64KB / 4 banks";
		rom_sizes[0x02] = "128KB / 8 banks";
		rom_sizes[0x03] = "256KB / 16 banks";
		rom_sizes[0x04] = "512KB / 32 banks";
		rom_sizes[0x05] = "1MB / 64 banks";
		rom_sizes[0x06] = "2MB / 128 banks";
		rom_sizes[0x52] = "1.1MB / 72 banks";
		rom_sizes[0x53] = "1.2MB / 80 banks";
		rom_sizes[0x54] = "1.5MB / 96 banks";

		map<uint8_t, int> rom_banks;
		rom_banks[0x00] = 2;
		rom_banks[0x01] = 4;
		rom_banks[0x02] = 8;
		rom_banks[0x03] = 16;
		rom_banks[0x04] = 32;
		rom_banks[0x05] = 64;
		rom_banks[0x06] = 128;
		rom_banks[0x52] = 72;
		rom_banks[0x53] = 80;
		rom_banks[0x54] = 96;

		uint8_t rom = MEM.RAW[0x0148];
		if (rom_sizes.count(rom)) {
			printf("ROM:\t%s\n", rom_sizes[rom].c_str());
			assert(rom_banks[rom] == MEM.rom_size / 0x4000);
		} else {
			printf("Unknown rom size 0x%02X\n", type);
			exit(1);
		}

		map<uint8_t, string> ram_sizes;
		ram_sizes[0x00] = "None";
	  ram_sizes[0x01] = "2KB / 1 bank";
	  ram_sizes[0x02] = "8KB / 1 bank";
	  ram_sizes[0x03] = "32KB / 4 banks";
	  ram_sizes[0x04] = "128KB / 16 banks";

	  map<uint8_t, int> ram_banks;
	  ram_banks[0x00] = 0;
	  ram_banks[0x01] = 1;
	  ram_banks[0x02] = 1;
	  ram_banks[0x03] = 4;
	  ram_banks[0x04] = 16;

	  uint8_t ram = MEM.RAW[0x0149];
		if (ram_sizes.count(ram)) {
			printf("RAM:\t%s\n", ram_sizes[ram].c_str());
			MEM.RAM_BANKS = (uint8_t*)malloc(0x1000 * ram_banks[ram]);
			if (ram_banks[ram])
				MEM.loadRAMBank(0); // TODO: if there is no ram banks?
		} else {
			printf("Unknown ram size 0x%02X\n", type);
			exit(1);
		}

		printf("CPL:\t0x%02X\n", MEM.RAW[0x014D]);
		printf("CHK:\t0x%02X 0x%02X\n", MEM.RAW[0x014E], MEM.RAW[0x014F]);
}