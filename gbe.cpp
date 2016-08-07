#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include "gbe.h"
#include "gpu.h"
#include "window.h"

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
			silent = false,
			breakpoint = false,
			stepping = false;

  uint16_t log_mem_addr = 0;
  uint16_t breakpoint_addr = 0;

  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          {"silent", no_argument, &silent, 1},
          {"rw",     no_argument, &log_register_words, 1},
          {"rb",     no_argument, &log_register_bytes, 1},
          {"gpu",    no_argument, &log_gpu, 1},
          {"flags",  no_argument, &log_flags, 1},

          {"breakpoint",  required_argument, 0, 'b'},
          {"step",  required_argument, 0, 's'},
          {"memory",      required_argument, 0, 'm'},
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
          printf ("option -b with value `%s'\n", optarg);
          breakpoint_addr = std::stoi(optarg,0,0);
          break;

        case 'm':
          printf ("option -m with value `%s'\n", optarg);
          log_mem_addr = std::stoi(optarg,0,0);
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

		if (stepping || (breakpoint && REG.PC == breakpoint_addr)) {
			exit(1);
			char opt = getchar();
			if (opt == 'r') stepping = false;
			if (opt == 's') stepping = true;
		}
		
		uint8_t opcode = MEM.readByte(REG.PC);
		instruction instr = instructions[opcode];

		if (log_register_bytes)
		printf("A %02X F %02X B %02X C %02X D %02X E %02X H %02X L %02X\n",
					 REG.A, REG.F, REG.B, REG.C, REG.D, REG.E, REG.H, REG.L);
		if (log_register_words)
		printf("AF %04X BC %04X DE %04X HL %04X SP %04X PC %04X\n",
					 REG.AF, REG.BC, REG.DE, REG.HL, REG.SP, REG.PC);
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

		if (!silent) {
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

		instr.fn();
		GPU.update();

		if (REG.IME && (*MEM.IE & *MEM.IF)) {

			// TODO: handle interrupt
		}

	}
}