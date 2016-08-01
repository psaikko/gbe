#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gbe.h"

unsigned char ROM[1010101];

void readROMFile(char * filename) {
	FILE* romfile; 
	romfile = fopen(filename, "rb");

	assert(romfile);

	fseek(romfile, 0, SEEK_END);
	size_t n = ftell(romfile);
	fseek(romfile, 0, SEEK_SET);
	fread(ROM,n,1,romfile);
	fclose(romfile);
}

void readBIOSFile(char * filename) {
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

	while (1) {
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

		instr.fn();
		REG.PC += (1 + instr.argw);
	}
}