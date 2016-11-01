#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "gbe.h"
#include "gpu.h"
#include "window.h"
#include "interrupts.h"
#include "timer.h"

using namespace std;

void printRegisters(bool words) {
	if (!words) {
		printf("A %02X F %02X B %02X C %02X D %02X E %02X H %02X L %02X\n",
						 REG.A, REG.F, REG.B, REG.C, REG.D, REG.E, REG.H, REG.L);
	} else {
		printf("AF %04X BC %04X DE %04X HL %04X SP %04X PC %04X (HL) %02X\n",
					 REG.AF, REG.BC, REG.DE, REG.HL, REG.SP, REG.PC, MEM.readByte(REG.HL));
	}
}

void printInstruction() {
	uint8_t opcode = MEM.readByte(REG.PC);
	instruction instr = instructions[opcode];
	printf("Instruction 0x%02X at 0x%04X: ", opcode, REG.PC);
	if (instr.argw == 0)
		printf(instr.name);
	else if (instr.argw == 1)
		printf(instr.name, MEM.readByte(REG.PC+1));
	else if (instr.argw == 2)
		printf(instr.name, MEM.readWord(REG.PC+1));
	printf("\n");

	if (opcode == 0xCB) {
		uint8_t ext_opcode = MEM.readByte(REG.PC+1);
		printf("        Ext 0x%02X at 0x%04X: ", ext_opcode, REG.PC+1);
		printf(ext_instructions[MEM.readByte(REG.PC+1)].name);
		printf("\n");
	}
}

void readROMFile(const string filename) {
	ifstream romfile(filename, ios::binary);
	romfile.read((char *)MEM.ROM0, 0x4000);
	romfile.read((char *)MEM.ROM1, 0x4000);
	romfile.close();
}

void readBIOSFile(const string filename) {
	ifstream biosfile(filename, ios::binary);
	biosfile.read((char *)MEM.BIOS, 256);
	biosfile.close();
}

int main(int argc, char ** argv) {

	int log_register_bytes = false, 
			log_register_words = false, 
			log_flags = false,
			log_gpu = false,
			log_instructions = false,
			breakpoint = false,
			mem_breakpoint = false,
			stepping = false,
			load_bios = false,
			load_rom = false;

	string romfile, biosfile;

  uint16_t breakpoint_addr = 0;

  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          {"rw",     no_argument, &log_register_words, 1},
          {"rb",     no_argument, &log_register_bytes, 1},
          {"gpu",    no_argument, &log_gpu, 1},

          {"instructions", no_argument, 0, 'i'},
          {"flags", no_argument, 0, 'f'},
          {"bios", required_argument, 0, 'B'},
          {"rom", required_argument, 0, 'R'},
          {"breakpoint", required_argument, 0, 'b'},
          {"step",       required_argument, 0, 's'},
          {"memory-breakpoint",     required_argument, 0, 'M'},
          {0, 0, 0, 0}
        };

      int option_index = 0;
      c = getopt_long (argc, argv, "s:b:ifB:R:M:", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1) break;

      switch (c) {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0) break;
          printf ("option %s", long_options[option_index].name);
          if (optarg) printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'B':
        	load_bios = true;
        	biosfile = string(optarg);
        	break;

        case 'R':
        	load_rom = true;
        	romfile = string(optarg);
        	break;

        case 'b':
        	breakpoint = true;
          printf ("option -b with value `%s'\n", optarg);
          breakpoint_addr = std::stoi(optarg,0,0);
          break;

        case 'M':
        	printf ("option -M with value `%s'\n", optarg);
          mem_breakpoint = true;
          MEM.break_addr = std::stoi(optarg,0,0);
          break;	

        case 'f':
        	log_flags = true;
        	break;

        case 'i':
        	log_instructions = true;
        	break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
       }
    }

  if (optind < argc) {
    fprintf(stderr, "[warning]: Unhandled args");
    while (optind < argc) fprintf (stderr, " %s", argv[optind++]);
    fprintf(stderr, "\n");
  }

  if (load_bios) {
  	readBIOSFile(biosfile);
  } else {
  	REG.AF = 0x01B0;
  	REG.BC = 0x0013;
  	REG.DE = 0x00D8;
  	REG.HL = 0x014D;
  	REG.SP = 0xFFFE;
  	REG.PC = 0x0100;
  	*MEM.BIOS_OFF = 1;
  }

  if (load_rom) {
  	readROMFile(romfile);

  	// cart data reference http://www.devrs.com/gb/files/gbspec.txt

  	char rom_name[16];
  	memcpy(rom_name, &MEM.RAW[0x0134], 16);
  	printf("NAME:\t%s\n", rom_name);
  	if (MEM.RAW[0x0143] == 0x80) printf("GBC\n");
  	if (MEM.RAW[0x0146] == 0x00) printf("GB\n");
  	if (MEM.RAW[0x0146] == 0x03) printf("SGB\n");

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
			printf("TYPE:\t%s\n", rom_types[type].c_str());
		} else {
			printf("Unknown type 0x%02X\n", type);
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

		uint8_t rom = MEM.RAW[0x0148];
		if (rom_sizes.count(rom)) {
			printf("ROM:\t%s\n", rom_sizes[rom].c_str());
		} else {
			printf("Unknown rom size 0x%02X\n", type);
		}

		map<uint8_t, string> ram_sizes;
		ram_sizes[0x00] = "None";
	  ram_sizes[0x01] = "2KB / 1 bank";
	  ram_sizes[0x02] = "8KB / 1 bank";
	  ram_sizes[0x03] = "32KB / 4 banks";
	  ram_sizes[0x04] = "128KB / 16 banks";

	  uint8_t ram = MEM.RAW[0x0149];
		if (ram_sizes.count(ram)) {
			printf("RAM:\t%s\n", ram_sizes[ram].c_str());
		} else {
			printf("Unknown ram size 0x%02X\n", type);
		}

		printf("CPL:\t0x%02X\n", MEM.RAW[0x014D]);
		printf("CHK:\t0x%02X 0x%02X\n", MEM.RAW[0x014E], MEM.RAW[0x014F]);
  } else if (!load_bios) {
  	printf("No rom (-R) or bios (-B) loaded. Exiting.\n");
  	exit(0);
  }

	WINDOW.init();

	while (1) {

		glfwPollEvents();
		key_state = 0;
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      key_state |= KEY_LEFT;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      key_state |= KEY_RIGHT;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_UP) == GLFW_PRESS) {
      key_state |= KEY_UP;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      key_state |= KEY_DOWN;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_Z) == GLFW_PRESS) {
    	key_state |= KEY_A;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_X) == GLFW_PRESS) {
    	key_state |= KEY_B;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_C) == GLFW_PRESS) {
    	key_state |= KEY_START;
    }
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_V) == GLFW_PRESS) {
    	key_state |= KEY_SELECT;
    }

    // Check if the ESC key was pressed or the window was closed
    if (glfwGetKey(WINDOW.game_window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwWindowShouldClose(WINDOW.game_window) == 1) {
    	break;
    }

		bool is_breakpoint = (breakpoint && (REG.PC == breakpoint_addr)) ||
												 (mem_breakpoint && (MEM.at_breakpoint));
		MEM.at_breakpoint = false;
		
		uint8_t opcode = MEM.readByte(REG.PC);
		instruction instr = instructions[opcode];

		if (log_register_bytes) printRegisters(false);
		if (log_register_words) printRegisters(true);
		if (log_flags) {
			printf("Z %1d N %1d H %1d C %1d\n",
							get_flag(FLAG_Z), get_flag(FLAG_N), get_flag(FLAG_H), get_flag(FLAG_C));
			printf("LCD_CTRL %02X LCD_STAT %02x SCAN_LN %02X PLT %02X BIOS_OFF %1X\n",
							*MEM.LCD_CTRL, *MEM.LCD_STAT, *MEM.SCAN_LN, *MEM.BG_PLT, *MEM.BIOS_OFF);
			printf("IME %X IE %02X IF %02X\n", REG.IME, *MEM.IE, *MEM.IF);
		}
		if (log_gpu) {
			printf("GPU CLK: 0x%04X LINE: 0x%02X\n", 
						  GPU.clk, *MEM.SCAN_LN);
		}

		if (log_instructions) printInstruction();

		if (stepping || is_breakpoint) {
			WINDOW.refresh_debug();
			stepping = true;
			bool parsing = true;
			bool more = false;
			printf("%04X> ", REG.PC);
			while (parsing) {
				switch (getchar()) {
					case 'R':
						// toggle register word logging
						log_register_words = !log_register_words;
						break;
					case 'r':
						// toggle register byte logging
						log_register_bytes = !log_register_bytes;
						break;
					case 'i':
						// toggle instruction logging
						log_instructions = !log_instructions;
						break;
					case 'I':
						// display current state
						printRegisters(true);
						printInstruction();
						more = true;
						break;
					case 's':
						// turn on stepping
						stepping = true;
						break;
					case 'c':
						// turn off stepping
						stepping = false;
						break;
					case 'q':
						// quit
						exit(1);
					case 'n':
						// skip to instruction at PC+1
						stepping = false;
						breakpoint = true;
						breakpoint_addr = REG.PC + (1 + instr.argw);
						break;
					case 'b':
						// set breakpoint
						stepping = false;
						breakpoint = true;
						scanf("%hX", &breakpoint_addr);
						break;
					case 'M':
						// set memory breakpoint
						stepping = false;
						mem_breakpoint = true;
						scanf("%hX", &MEM.break_addr);
						break;
					case 'd':
						// dump memory range
						uint16_t addr;
						uint16_t len;
						scanf("%hX %hX", &addr, &len);
						for (uint16_t i = 0; i < len; ++i) {
							printf("%02X ", MEM.RAW[addr+i]);
							if ((i + 1) % 8 == 0) printf("\n");
						}
						more = true;
						break;
					case 'f':
						// toggle flag logging
						log_flags = !log_flags;
						break;
					case '\n':
						parsing = more ? true : false;
						more = false;
						if (parsing) printf("%04X> ", REG.PC);
						break;
				}
			}
		}
	
		if (!REG.HALT) instr.fn();
		GPU.update();

		TIMER.update();

		handle_interrupts();

	}
}