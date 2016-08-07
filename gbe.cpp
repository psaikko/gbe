#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include "gbe.h"
#include "gpu.h"
#include "window.h"

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

void readROMFile(const char * filename) {
	FILE* romfile; 
	romfile = fopen(filename, "rb");

	fread(MEM.ROM0,0x3fff,1,romfile);
	fread(MEM.ROM1,0x3fff,1,romfile);
	fclose(romfile);
}

void readBIOSFile(const char * filename) {
	FILE* biosfile; 
	biosfile = fopen(filename, "rb");

	assert(biosfile);

	fseek(biosfile, 0, SEEK_END);
	size_t n = ftell(biosfile);
	assert(n == 256);
	fseek(biosfile, 0, SEEK_SET);
	fread(MEM.BIOS,n,1,biosfile);

	fclose(biosfile);
}

int main(int argc, char ** argv) {
	readBIOSFile("rom.bin");
	readROMFile("tetris.gb");

	int log_register_bytes = false, 
			log_register_words = false, 
			log_flags = false,
			log_mem = false,
			log_gpu = false,
			log_instructions = false,
			breakpoint = false,
			stepping = false;

  uint16_t log_mem_addr = 0;
  uint16_t breakpoint_addr = 0;

  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          {"rw",     no_argument, &log_register_words, 1},
          {"rb",     no_argument, &log_register_bytes, 1},
          {"gpu",    no_argument, &log_gpu, 1},
          {"flags",  no_argument, &log_flags, 1},

          {"instructions", no_argument, 0, 'i'},
          {"breakpoint", required_argument, 0, 'b'},
          {"step",       required_argument, 0, 's'},
          {"memory",     required_argument, 0, 'm'},
          {0, 0, 0, 0}
        };

      int option_index = 0;
      c = getopt_long (argc, argv, "s:b:m:", long_options, &option_index);

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

        case 'b':
        	breakpoint = true;
          printf ("option -b with value `%s'\n", optarg);
          breakpoint_addr = std::stoi(optarg,0,0);
          break;

        case 'm':
          printf ("option -m with value `%s'\n", optarg);
          log_mem = true;
          log_mem_addr = std::stoi(optarg,0,0);
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

  if (optind < argc)
  {
    fprintf(stderr, "[warning]: Unhandled args");
    while (optind < argc) fprintf (stderr, " %s", argv[optind++]);
    fprintf(stderr, "\n");
  }

	WINDOW.init();

	while (1) {

		bool is_breakpoint = breakpoint && (REG.PC == breakpoint_addr);
		
		uint8_t opcode = MEM.readByte(REG.PC);
		instruction instr = instructions[opcode];

		if (log_register_bytes) printRegisters(false);
		if (log_register_words) printRegisters(true);
		if (log_flags)
		printf("Z %1d N %1d H %1d C %1d\n",
						get_flag(FLAG_Z), get_flag(FLAG_N), get_flag(FLAG_H), get_flag(FLAG_C));
		if (log_mem) {
			printf("0x%02X\n", MEM.readByte(log_mem_addr));
		}
		if (log_gpu) {
			printf("GPU CLK: 0x%04X  MODE: %d  LINE: 0x%02X\n", 
						  GPU.clk, GPU.mode, *MEM.SCAN_LN);
		}

		if (log_instructions) printInstruction();

		if (stepping || is_breakpoint) {
			stepping = true;
			bool parsing = true;
			bool more = false;
			printf("%04X> ", REG.PC);
			while (parsing) {
				switch (getchar()) {
					case 'R':
						log_register_words = !log_register_words;
						break;
					case 'r':
						log_register_bytes = !log_register_bytes;
						break;
					case 'i':
						log_instructions = !log_instructions;
						break;
					case 'I':
						printRegisters(true);
						printInstruction();
						more = true;
						break;
					case 's':
						stepping = true;
						break;
					case 'c':
						stepping = false;
						break;
					case 'q':
						exit(1);
					case 'b':
						stepping = false;
						breakpoint = true;
						scanf("%hX", &breakpoint_addr);
						break;
					case 'f':
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

		instr.fn();
		GPU.update();

		if (REG.IME && (*MEM.IE & *MEM.IF)) {

			// TODO: handle interrupt
		}

	}
}