#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gbe.h"

unsigned char ROM[1010101];

instruction instructions[256] = {
	{"NOP", 0, NULL},         // 0x00
	{"LD BC, 0x%04X", 2, NULL},   // 0x01
	{"LD (BC), A", 0, NULL},  // 0x02
	{"INC BC", 0, NULL},      // 0x03
	{"INC B", 0, NULL},       // 0x04
	{"DEC B", 0, NULL},       // 0x05
	{"LD B, 0x%02X", 1, NULL},     // 0x06
	{"RLC A", 0, NULL},       // 0x07
	{"LD (0x%04X), SP", 2, NULL}, // 0x08
	{"ADD HL, BC", 0, NULL},  // 0x09
	{"LD A, (BC)", 0, NULL},  // 0x0A
	{"DEC BC", 0, NULL},      // 0x0B
	{"INC C", 0, NULL},       // 0x0C
	{"DEC C", 0, NULL},       // 0x0D
	{"LD C, 0x%02X", 1, NULL},     // 0x0E
	{"RRC A", 0, NULL},       // 0x0F

	{"STOP", 0, NULL},        // 0x10
	{"LD DE, 0x%04X", 2, NULL},   // 0x11
	{"LD (DE), A", 0, NULL},  // 0x12
	{"INC DE", 0, NULL},      // 0x13
	{"INC D", 0, NULL},       // 0x14
	{"DEC D", 0, NULL},       // 0x15
	{"LD D, 0x%02X", 1, NULL},     // 0x16
	{"RL A", 0, NULL},        // 0x17
	{"JR 0x%02X", 1, NULL},        // 0x18
	{"ADD HL, DE", 0, NULL},  // 0x19
	{"LD A, (DE)", 0, NULL},  // 0x1A
	{"DEC DE", 0, NULL},      // 0x1B
	{"INC E", 0, NULL},       // 0x1C
	{"DEC E", 0, NULL},       // 0x1D
	{"LD E, 0x%02X", 1, NULL},     // 0x1E
	{"RR A", 0, NULL},        // 0x1F

	{"JR NZ, n", 0, NULL},    // 0x20
	{"LD HL, 0x%04X", 2, NULL},   // 0x21
	{"LD (DE), A", 0, NULL},  // 0x22
	{"INC HL", 0, NULL},      // 0x23
	{"INC H", 0, NULL},       // 0x24
	{"DEC H", 0, NULL},       // 0x25
	{"LD H, 0x%02X", 1, NULL},     // 0x26
	{"DAA", 0, NULL},         // 0x27
	{"JR Z, 0x%02X", 1, NULL},     // 0x28
	{"ADD HL, HL", 0, NULL},  // 0x29
	{"LDI A, (HL)", 0, NULL}, // 0x2A
	{"DEC HL", 0, NULL},      // 0x2B
	{"INC L", 0, NULL},       // 0x2C
	{"DEC L", 0, NULL},       // 0x2D
	{"LD L, 0x%02X", 1, NULL},     // 0x2E
	{"CPL", 0, NULL},         // 0x2F
 
	{"JR NC, 0x%02X", 1, NULL},    // 0x30
	{"LD SP, 0x%04X", 2, NULL},   // 0x31
	{"LDD (HL), A", 0, NULL}, // 0x32
	{"INC SP", 0, NULL},      // 0x33
	{"INC (HL)", 0, NULL},    // 0x34
	{"DEC (HL)", 0, NULL},    // 0x35
	{"LD (HL), 0x%02X", 1, NULL},  // 0x36
	{"SCF", 0, NULL},         // 0x37
	{"JR C, 0x%02X", 1, NULL},     // 0x38
	{"ADD HL, SP", 0, NULL},  // 0x39
	{"LDD A, (HL)", 0, NULL}, // 0x3A
	{"DEC SP", 0, NULL},      // 0x3B
	{"INC A", 0, NULL},       // 0x3C
	{"DEC A", 0, NULL},       // 0x3D
	{"LD A, 0x%02X", 1, NULL},     // 0x3E
	{"CCF", 0, NULL},         // 0x3F

	{"LD B, B", 0, NULL},     // 0x40
	{"LD B, C", 0, NULL},     // 0x41
	{"LD B, D", 0, NULL},     // 0x42
	{"LD B, E", 0, NULL},     // 0x43
	{"LD B, H", 0, NULL},     // 0x44
	{"LD B, L", 0, NULL},     // 0x45
	{"LD B, (HL)", 0, NULL},  // 0x46
	{"LD B, A", 0, NULL},     // 0x47
	{"LD C, B", 0, NULL},     // 0x47
	{"LD C, C", 0, NULL},     // 0x48
	{"LD C, D", 0, NULL},     // 0x49
	{"LD C, E", 0, NULL},     // 0x4A
	{"LD C, H", 0, NULL},     // 0x4C
	{"LD C, L", 0, NULL},     // 0x4D
	{"LD C, (HL)", 0, NULL},  // 0x4E
	{"LD C, A", 0, NULL},     // 0x4F

	{"LD D, B", 0, NULL},     // 0x50
	{"LD D, C", 0, NULL},     // 0x51
	{"LD D, D", 0, NULL},     // 0x52
	{"LD D, E", 0, NULL},     // 0x53
	{"LD D, H", 0, NULL},     // 0x54
	{"LD D, L", 0, NULL},     // 0x55
	{"LD D, (HL)", 0, NULL},  // 0x56
	{"LD D, A", 0, NULL},     // 0x57
	{"LD E, B", 0, NULL},     // 0x57
	{"LD E, C", 0, NULL},     // 0x58
	{"LD E, D", 0, NULL},     // 0x59
	{"LD E, E", 0, NULL},     // 0x5A
	{"LD E, H", 0, NULL},     // 0x5C
	{"LD E, L", 0, NULL},     // 0x5D
	{"LD E, (HL)", 0, NULL},  // 0x5E
	{"LD E, A", 0, NULL},     // 0x5F

	{"LD H, B", 0, NULL},     // 0x60
	{"LD H, C", 0, NULL},     // 0x61
	{"LD H, D", 0, NULL},     // 0x62
	{"LD H, E", 0, NULL},     // 0x63
	{"LD H, H", 0, NULL},     // 0x64
	{"LD H, L", 0, NULL},     // 0x65
	{"LD H, (HL)", 0, NULL},  // 0x66
	{"LD H, A", 0, NULL},     // 0x67
	{"LD L, B", 0, NULL},     // 0x67
	{"LD L, C", 0, NULL},     // 0x68
	{"LD L, D", 0, NULL},     // 0x69
	{"LD L, E", 0, NULL},     // 0x6A
	{"LD L, H", 0, NULL},     // 0x6C
	{"LD L, L", 0, NULL},     // 0x6D
	{"LD L, (HL)", 0, NULL},  // 0x6E
	{"LD L, A", 0, NULL},     // 0x6F

	{"LD (HL), B", 0, NULL},  // 0x70
	{"LD (HL), C", 0, NULL},  // 0x71
	{"LD (HL), D", 0, NULL},  // 0x72
	{"LD (HL), E", 0, NULL},  // 0x73
	{"LD (HL), H", 0, NULL},  // 0x74
	{"LD (HL), L", 0, NULL},  // 0x75
	{"HALT", 0, NULL},        // 0x76
	{"LD (HL), A", 0, NULL},  // 0x77
	{"LD A, B", 0, NULL},     // 0x77
	{"LD A, C", 0, NULL},     // 0x78
	{"LD A, D", 0, NULL},     // 0x79
	{"LD A, E", 0, NULL},     // 0x7A
	{"LD A, H", 0, NULL},     // 0x7C
	{"LD A, L", 0, NULL},     // 0x7D
	{"LD A, (HL)", 0, NULL},  // 0x7E
	{"LD A, A", 0, NULL},     // 0x7F

	{"ADD A, B", 0, NULL},    // 0x80
	{"ADD A, C", 0, NULL},    // 0x81
	{"ADD A, D", 0, NULL},    // 0x82
	{"ADD A, E", 0, NULL},    // 0x83
	{"ADD A, H", 0, NULL},    // 0x84
	{"ADD A, L", 0, NULL},    // 0x85
	{"ADD A, (HL)", 0, NULL}, // 0x86
	{"ADD A, A", 0, NULL},    // 0x87
	{"ADC A, B", 0, NULL},    // 0x87
	{"ADC A, C", 0, NULL},    // 0x88
	{"ADC A, D", 0, NULL},    // 0x89
	{"ADC A, E", 0, NULL},    // 0x8A
	{"ADC A, H", 0, NULL},    // 0x8C
	{"ADC A, L", 0, NULL},    // 0x8D
	{"ADC A, (HL)", 0, NULL}, // 0x8E
	{"ADC A, A", 0, NULL},    // 0x8F

	{"SUB A, B", 0, NULL},    // 0x90
	{"SUB A, C", 0, NULL},    // 0x91
	{"SUB A, D", 0, NULL},    // 0x92
	{"SUB A, E", 0, NULL},    // 0x93
	{"SUB A, H", 0, NULL},    // 0x94
	{"SUB A, L", 0, NULL},    // 0x95
	{"SUB A, (HL)", 0, NULL}, // 0x96
	{"SUB A, A", 0, NULL},    // 0x97
	{"SBC A, B", 0, NULL},    // 0x97
	{"SBC A, C", 0, NULL},    // 0x98
	{"SBC A, D", 0, NULL},    // 0x99
	{"SBC A, E", 0, NULL},    // 0x9A
	{"SBC A, H", 0, NULL},    // 0x9C
	{"SBC A, L", 0, NULL},    // 0x9D
	{"SBC A, (HL)", 0, NULL}, // 0x9E
	{"SBC A, A", 0, NULL},    // 0x9F

	{"AND B", 0, NULL},       // 0xA0
	{"AND C", 0, NULL},       // 0xA1
	{"AND D", 0, NULL},       // 0xA2
	{"AND E", 0, NULL},       // 0xA3
	{"AND H", 0, NULL},       // 0xA4
	{"AND L", 0, NULL},       // 0xA5
	{"AND (HL)", 0, NULL},    // 0xA6
	{"AND A", 0, NULL},       // 0xA7
	{"XOR B", 0, NULL},       // 0xA7
	{"XOR C", 0, NULL},       // 0xA8
	{"XOR D", 0, NULL},       // 0xA9
	{"XOR E", 0, NULL},       // 0xAA
	{"XOR H", 0, NULL},       // 0xAC
	{"XOR L", 0, NULL},       // 0xAD
	{"XOR (HL)", 0, NULL},    // 0xAE
	{"XOR A", 0, NULL},       // 0xAF

	{"OR B", 0, NULL},        // 0xB0
	{"OR C", 0, NULL},        // 0xB1
	{"OR D", 0, NULL},        // 0xB2
	{"OR E", 0, NULL},        // 0xB3
	{"OR H", 0, NULL},        // 0xB4
	{"OR L", 0, NULL},        // 0xB5
	{"OR (HL)", 0, NULL},     // 0xB6
	{"OR A", 0, NULL},        // 0xB7
	{"CP B", 0, NULL},        // 0xB7
	{"CP C", 0, NULL},        // 0xB8
	{"CP D", 0, NULL},        // 0xB9
	{"CP E", 0, NULL},        // 0xBA
	{"CP H", 0, NULL},        // 0xBC
	{"CP L", 0, NULL},        // 0xBD
	{"CP (HL)", 0, NULL},     // 0xBE
	{"CP A", 0, NULL},        // 0xBF

	{"RET NZ", 0, NULL},      // 0xC0
	{"POP BC", 0, NULL},      // 0xC1
	{"JP NZ, 0x%04X", 2, NULL},   // 0xC2
	{"JP 0x%04X", 2, NULL},       // 0xC3
	{"CALL NZ, 0x%04X", 2, NULL}, // 0xC4
	{"PUSH BC", 0, NULL},     // 0xC5
	{"ADD A, 0x%02X", 1, NULL},    // 0xC6
	{"RST 0", 0, NULL},       // 0xC7
	{"RET Z", 0, NULL},       // 0xC7
	{"RET", 0, NULL},         // 0xC8
	{"JP Z, 0x%04X", 2, NULL},    // 0xC9
	{"Ext Op", 0, NULL},         // 0xCA
	{"CALL Z, 0x%04X", 2, NULL},  // 0xCC
	{"CALL 0x%04X", 2, NULL},     // 0xCD
	{"ADC A, 0x%02X", 1, NULL},    // 0xCE
	{"RST 8", 0, NULL},       // 0xCF

};

registers REG;
memory MEM;

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
		if (instr.fn) {
			instr.fn();
		} else {
			printf("Missing instruction 0x%02X at 0x%04X: ", opcode, REG.PC);
			if (instr.argw == 0)
				printf(instr.name);
			else if (instr.argw == 1)
				printf(instr.name, MEM.readByte(REG.PC+1));
			else if (instr.argw == 2)
				printf(instr.name, MEM.readByte(REG.PC+1));
			printf("\n");
			exit(1);
		}
	}
}