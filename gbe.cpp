#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include <fstream>

#include "gbe.h"
#include "gpu.h"
#include "window.h"
#include "interrupts.h"

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
  } else if (!load_bios) {
  	printf("No rom (-R) or bios (-B) loaded. Exiting.\n");
  	exit(0);
  }

	WINDOW.init();

	while (1) {

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
		handle_interrupts();

	}
}