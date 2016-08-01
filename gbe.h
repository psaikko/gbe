#pragma once
#include <inttypes.h>
#include <functional>
#include <algorithm>

typedef void (*instr_fn)(void);

typedef struct {
	struct {
		union {
			uint16_t AF;
			struct {
				uint8_t F;
				uint8_t A;
			};
		};
	};

	struct {
		union {
			uint16_t BC;
			struct {
				uint8_t C;
				uint8_t B;
			};
		};
	};

	struct {
		union {
			uint16_t DE;
			struct {
				uint8_t E;
				uint8_t D;
			};
		};
	};

	struct {
		union {
			uint16_t HL;
			struct {
				uint8_t L;
				uint8_t H;
			};
		};
	};

	uint16_t SP;
	uint16_t PC;
	uint64_t T;
} registers;

registers REG;

uint8_t FLAG_C = 0x10;
uint8_t FLAG_H = 0x20;
uint8_t FLAG_N = 0x40;
uint8_t FLAG_Z = 0x80;

void set_flag(uint8_t flag) {
	REG.F |= flag;
}

void set_flag_cond(uint8_t flag, bool on) {
	if (on) REG.F |= flag;
	else    REG.F &= ~flag;
}

void unset_flag(uint8_t flag) {
	REG.F &= ~flag;
}

typedef struct {
	uint8_t ROM0[16384];  // [0000-3FFF] 
	uint8_t ROM1[16384];  // [4000-7FFF]
	uint8_t grRAM[8192];  // [8000-9FFF] 
	uint8_t extRAM[8192]; // [A000-BFFF]
	uint8_t RAM[8192];    // [C000-DFFF]
	uint8_t _RAM[7680];   // [E000-FDFF]
	uint8_t SPR[255];     // [FE00-FE9F] // 160 bytes?
	uint8_t IO[128];      // [FF00-FF7F] 
	uint8_t ZERO[128];    // [FF80-FFFF]

	bool bios = true;
	uint8_t BIOS[256]; 

	uint8_t* getPtr(uint16_t addr) {

	}

	uint8_t readByte(uint16_t addr) {
		return BIOS[addr];
	}

	uint16_t readWord(uint16_t addr) {
		return (uint16_t)(BIOS[addr+1]) << 8 | (uint16_t)(BIOS[addr]);
	}

	void writeByte(uint16_t addr, uint8_t val) {
		assert(0);
	}

	void writeWord(uint16_t addr, uint16_t val) {
		assert(0);	
	}
} memory;

memory MEM;

void nop() {
	REG.T += 4;
}

// 8-bit arithmetic

void inc_rb(uint8_t * ptr) {
	(*ptr)++;

	unset_flag(FLAG_N);
	set_flag_cond(FLAG_H, (*ptr) & 0x0F == 0);
	set_flag_cond(FLAG_Z, (*ptr) == 0);
	REG.T += 4;
}

void inc_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	MEM.writeByte(REG.HL, val + 1);

	unset_flag(FLAG_N);
	set_flag_cond(FLAG_H, val & 0x0F == 0x0F);
	set_flag_cond(FLAG_Z, (val + 1) == 0);

	REG.T += 12;
}

void xor_rb(uint8_t * from) {
	REG.A ^= *from;
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 4;
} 

void xor_n() {
	REG.A ^= MEM.readByte(REG.PC + 1);
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
}

void xor_atHL() {
	uint16_t addr = MEM.readWord(REG.HL); 
	REG.A ^= MEM.readByte(addr);
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
}

void or_rb(uint8_t * from) {
	REG.A |= *from;
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 4;
} 

void or_n() {
	REG.A |= MEM.readByte(REG.PC + 1);
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
}

void or_atHL() {
	uint16_t addr = MEM.readWord(REG.HL);
	REG.A |= MEM.readByte(addr);
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
}

void and_rb(uint8_t * from) {
	REG.A &= *from;
	
	unset_flag(FLAG_N | FLAG_C);
	set_flag(FLAG_H);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 4;
} 

void and_n() {
	REG.A &= MEM.readByte(REG.PC + 1);
	
	unset_flag(FLAG_N | FLAG_C);
	set_flag(FLAG_H);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
}

void and_atHL() {
	uint16_t addr = MEM.readWord(REG.HL);
	REG.A &= MEM.readByte(addr);
	
	unset_flag(FLAG_N | FLAG_C);
	set_flag(FLAG_H);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
}

void cp_rb(uint8_t * from) {

	uint8_t e = *from;

	set_flag(FLAG_N);
	set_flag_cond(FLAG_Z, e == REG.A);
	set_flag_cond(FLAG_C, e > REG.A);
	set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

	REG.T += 4;
} 

void cp_n() {

	uint8_t e = MEM.readByte(REG.PC + 1);

	set_flag(FLAG_N);
	set_flag_cond(FLAG_Z, e == REG.A);
	set_flag_cond(FLAG_C, e > REG.A);
	set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

	REG.T += 8;
}

void cp_atHL() {

	uint8_t e = MEM.readByte(MEM.readWord(REG.HL));
	
	set_flag(FLAG_N);
	set_flag_cond(FLAG_Z, e == REG.A);
	set_flag_cond(FLAG_C, e > REG.A);
	set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

	REG.T += 8;
}

// 16-bit arithmetic

void inc_rw(uint16_t * ptr) {
	(*ptr)++;
	REG.T += 8;	
}

// 8-bit loads

void ld_rb_rb(uint8_t *to, uint8_t *from) {
	(*to) = (*from);
	REG.T += 4;	
}

void ld_rb_n(uint8_t *to) {
	(*to) = MEM.readByte(REG.PC + 1);
	REG.T += 8;	
}

void ld_rb_atHL(uint8_t *to) {
	(*to) = MEM.readByte(REG.HL);
	REG.T += 12;		
}

void ld_atHL_rb(uint8_t *from) {
	MEM.writeByte(REG.HL, *from);
	REG.T += 8;
}

void ld_atHL_n() {
	MEM.writeByte(REG.HL, MEM.readByte(REG.PC + 1));
	REG.T += 12;
}

// 16-bit loads

void ld_rw_nn(uint16_t* to) {
	*to = MEM.readWord(REG.PC + 1);
	REG.T += 12;
}

void ld_atnn_SP() {
	uint16_t addr = MEM.readWord(REG.PC + 1);
	MEM.writeWord(addr, REG.SP);
	REG.T += 20;
}

void ld_SP_HL() {
	REG.SP = REG.HL;
	REG.T += 8;
}



typedef struct {
	char name[16];
	uint8_t argw;
	std::function<void()> fn;
} instruction;

std::function<void()> TODO = [&](){
	printf("Operation not implemented.\nContext:\n");
	uint16_t start = (REG.PC >= 10) ? REG.PC - 10 : 0;
	for (uint16_t i = start; i < REG.PC; ++i)
		printf("%02X ", MEM.readByte(start + i));
	printf("| %02X | ", MEM.readByte(REG.PC));
	uint16_t end = (REG.PC < 0xFFFF - 10) ? REG.PC + 10 : 0xFFFF;
	for (uint16_t i = REG.PC + 1; i < end; ++i)
		printf("%02X ", MEM.readByte(i));
	printf("\n");
	exit(1);
};

instruction instructions[256] = {
	{"NOP", 0, [&](){ nop(); }},            // 0x00
	{"LD BC, 0x%04X", 2, [&](){ ld_rw_nn(&REG.BC); }},  // 0x01
	{"LD (BC), A", 0, TODO},     // 0x02
	{"INC BC", 0, [&](){ inc_rw(&REG.BC); }},         // 0x03
	{"INC B", 0, [&](){ inc_rb(&REG.B); }},          // 0x04
	{"DEC B", 0, TODO},          // 0x05
	{"LD B, 0x%02X", 1, TODO},   // 0x06
	{"RLC A", 0, TODO},          // 0x07
	{"LD (0x%04X), SP", 2, [&](){ ld_atnn_SP(); }},// 0x08
	{"ADD HL, BC", 0, TODO},     // 0x09
	{"LD A, (BC)", 0, TODO},     // 0x0A
	{"DEC BC", 0, TODO},         // 0x0B
	{"INC C", 0, [&](){ inc_rb(&REG.C); }},          // 0x0C
	{"DEC C", 0, TODO},          // 0x0D
	{"LD C, 0x%02X", 1, TODO},   // 0x0E
	{"RRC A", 0, TODO},          // 0x0F

	{"STOP", 0, TODO},           // 0x10
	{"LD DE, 0x%04X", 2, [&](){ ld_rw_nn(&REG.DE); }},  // 0x11
	{"LD (DE), A", 0, TODO},     // 0x12
	{"INC DE", 0, [&](){ inc_rw(&REG.DE); } },         // 0x13
	{"INC D", 0, [&](){ inc_rb(&REG.D); }},          // 0x14
	{"DEC D", 0, TODO},          // 0x15
	{"LD D, 0x%02X", 1, TODO},   // 0x16
	{"RL A", 0, TODO},           // 0x17
	{"JR 0x%02X", 1, TODO},      // 0x18
	{"ADD HL, DE", 0, TODO},     // 0x19
	{"LD A, (DE)", 0, TODO},     // 0x1A
	{"DEC DE", 0, TODO},         // 0x1B
	{"INC E", 0, [&](){ inc_rb(&REG.E); }},          // 0x1C
	{"DEC E", 0, TODO},          // 0x1D
	{"LD E, 0x%02X", 1, TODO},   // 0x1E
	{"RR A", 0, TODO},           // 0x1F

	{"JR NZ, n", 0, TODO},       // 0x20
	{"LD HL, 0x%04X", 2, [&](){ ld_rw_nn(&REG.HL); }},  // 0x21
	{"LD (DE), A", 0, TODO},     // 0x22
	{"INC HL", 0, [&](){ inc_rw(&REG.HL); }},         // 0x23
	{"INC H", 0, [&](){ inc_rb(&REG.H); }},          // 0x24
	{"DEC H", 0, TODO},          // 0x25
	{"LD H, 0x%02X", 1, TODO},   // 0x26
	{"DAA", 0, TODO},            // 0x27
	{"JR Z, 0x%02X", 1, TODO},   // 0x28
	{"ADD HL, HL", 0, TODO},     // 0x29
	{"LDI A, (HL)", 0, TODO},    // 0x2A
	{"DEC HL", 0, TODO},         // 0x2B
	{"INC L", 0, [&](){ inc_rb(&REG.L); }},          // 0x2C
	{"DEC L", 0, TODO},          // 0x2D
	{"LD L, 0x%02X", 1, TODO},   // 0x2E
	{"CPL", 0, TODO},            // 0x2F
 
	{"JR NC, 0x%02X", 1, TODO},  // 0x30
	{"LD SP, 0x%04X", 2, [&](){ ld_rw_nn(&REG.SP); }},  // 0x31
	{"LDD (HL), A", 0, TODO},    // 0x32
	{"INC SP", 0, [&](){ inc_rw(&REG.SP); }},         // 0x33
	{"INC (HL)", 0, [&](){ inc_atHL(); }},       // 0x34
	{"DEC (HL)", 0, TODO},       // 0x35
	{"LD (HL), 0x%02X", 1, TODO},// 0x36
	{"SCF", 0, TODO},            // 0x37
	{"JR C, 0x%02X", 1, TODO},   // 0x38
	{"ADD HL, SP", 0, TODO},     // 0x39
	{"LDD A, (HL)", 0, TODO},    // 0x3A
	{"DEC SP", 0, TODO},         // 0x3B
	{"INC A", 0, TODO},          // 0x3C
	{"DEC A", 0, TODO},          // 0x3D
	{"LD A, 0x%02X", 1, TODO},   // 0x3E
	{"CCF", 0, TODO},            // 0x3F

	{"LD B, B", 0, TODO},        // 0x40
	{"LD B, C", 0, TODO},        // 0x41
	{"LD B, D", 0, TODO},        // 0x42
	{"LD B, E", 0, TODO},        // 0x43
	{"LD B, H", 0, TODO},        // 0x44
	{"LD B, L", 0, TODO},        // 0x45
	{"LD B, (HL)", 0, TODO},     // 0x46
	{"LD B, A", 0, TODO},        // 0x47
	{"LD C, B", 0, TODO},        // 0x47
	{"LD C, C", 0, TODO},        // 0x48
	{"LD C, D", 0, TODO},        // 0x49
	{"LD C, E", 0, TODO},        // 0x4A
	{"LD C, H", 0, TODO},        // 0x4C
	{"LD C, L", 0, TODO},        // 0x4D
	{"LD C, (HL)", 0, TODO},     // 0x4E
	{"LD C, A", 0, TODO},        // 0x4F

	{"LD D, B", 0, TODO},        // 0x50
	{"LD D, C", 0, TODO},        // 0x51
	{"LD D, D", 0, TODO},        // 0x52
	{"LD D, E", 0, TODO},        // 0x53
	{"LD D, H", 0, TODO},        // 0x54
	{"LD D, L", 0, TODO},        // 0x55
	{"LD D, (HL)", 0, TODO},     // 0x56
	{"LD D, A", 0, TODO},        // 0x57
	{"LD E, B", 0, TODO},        // 0x57
	{"LD E, C", 0, TODO},        // 0x58
	{"LD E, D", 0, TODO},        // 0x59
	{"LD E, E", 0, TODO},        // 0x5A
	{"LD E, H", 0, TODO},        // 0x5C
	{"LD E, L", 0, TODO},        // 0x5D
	{"LD E, (HL)", 0, TODO},     // 0x5E
	{"LD E, A", 0, TODO},        // 0x5F

	{"LD H, B", 0, TODO},        // 0x60
	{"LD H, C", 0, TODO},        // 0x61
	{"LD H, D", 0, TODO},        // 0x62
	{"LD H, E", 0, TODO},        // 0x63
	{"LD H, H", 0, TODO},        // 0x64
	{"LD H, L", 0, TODO},        // 0x65
	{"LD H, (HL)", 0, TODO},     // 0x66
	{"LD H, A", 0, TODO},        // 0x67
	{"LD L, B", 0, TODO},        // 0x67
	{"LD L, C", 0, TODO},        // 0x68
	{"LD L, D", 0, TODO},        // 0x69
	{"LD L, E", 0, TODO},        // 0x6A
	{"LD L, H", 0, TODO},        // 0x6C
	{"LD L, L", 0, TODO},        // 0x6D
	{"LD L, (HL)", 0, TODO},     // 0x6E
	{"LD L, A", 0, TODO},        // 0x6F

	{"LD (HL), B", 0, TODO},     // 0x70
	{"LD (HL), C", 0, TODO},     // 0x71
	{"LD (HL), D", 0, TODO},     // 0x72
	{"LD (HL), E", 0, TODO},     // 0x73
	{"LD (HL), H", 0, TODO},     // 0x74
	{"LD (HL), L", 0, TODO},     // 0x75
	{"HALT", 0, TODO},           // 0x76
	{"LD (HL), A", 0, TODO},     // 0x77
	{"LD A, B", 0, TODO},        // 0x77
	{"LD A, C", 0, TODO},        // 0x78
	{"LD A, D", 0, TODO},        // 0x79
	{"LD A, E", 0, TODO},        // 0x7A
	{"LD A, H", 0, TODO},        // 0x7C
	{"LD A, L", 0, TODO},        // 0x7D
	{"LD A, (HL)", 0, TODO},     // 0x7E
	{"LD A, A", 0, TODO},        // 0x7F

	{"ADD A, B", 0, TODO},       // 0x80
	{"ADD A, C", 0, TODO},       // 0x81
	{"ADD A, D", 0, TODO},       // 0x82
	{"ADD A, E", 0, TODO},       // 0x83
	{"ADD A, H", 0, TODO},       // 0x84
	{"ADD A, L", 0, TODO},       // 0x85
	{"ADD A, (HL)", 0, TODO},    // 0x86
	{"ADD A, A", 0, TODO},       // 0x87
	{"ADC A, B", 0, TODO},       // 0x87
	{"ADC A, C", 0, TODO},       // 0x88
	{"ADC A, D", 0, TODO},       // 0x89
	{"ADC A, E", 0, TODO},       // 0x8A
	{"ADC A, H", 0, TODO},       // 0x8C
	{"ADC A, L", 0, TODO},       // 0x8D
	{"ADC A, (HL)", 0, TODO},    // 0x8E
	{"ADC A, A", 0, TODO},       // 0x8F

	{"SUB A, B", 0, TODO},       // 0x90
	{"SUB A, C", 0, TODO},       // 0x91
	{"SUB A, D", 0, TODO},       // 0x92
	{"SUB A, E", 0, TODO},       // 0x93
	{"SUB A, H", 0, TODO},       // 0x94
	{"SUB A, L", 0, TODO},       // 0x95
	{"SUB A, (HL)", 0, TODO},    // 0x96
	{"SUB A, A", 0, TODO},       // 0x97
	{"SBC A, B", 0, TODO},       // 0x97
	{"SBC A, C", 0, TODO},       // 0x98
	{"SBC A, D", 0, TODO},       // 0x99
	{"SBC A, E", 0, TODO},       // 0x9A
	{"SBC A, H", 0, TODO},       // 0x9C
	{"SBC A, L", 0, TODO},       // 0x9D
	{"SBC A, (HL)", 0, TODO},    // 0x9E
	{"SBC A, A", 0, TODO},       // 0x9F

	{"AND B", 0, [&](){ and_rb(&REG.B); }},          // 0xA0
	{"AND C", 0, [&](){ and_rb(&REG.C); }},          // 0xA1
	{"AND D", 0, [&](){ and_rb(&REG.D); }},          // 0xA2
	{"AND E", 0, [&](){ and_rb(&REG.E); }},          // 0xA3
	{"AND H", 0, [&](){ and_rb(&REG.H); }},          // 0xA4
	{"AND L", 0, [&](){ and_rb(&REG.L); }},          // 0xA5
	{"AND (HL)", 0, [&](){ and_atHL(); }},       // 0xA6
	{"AND A", 0, [&](){ and_rb(&REG.A); }},          // 0xA7
	{"XOR B", 0, [&](){ xor_rb(&REG.B); }},          // 0xA7
	{"XOR C", 0, [&](){ xor_rb(&REG.C);}},          // 0xA8
	{"XOR D", 0, [&](){ xor_rb(&REG.D);}},          // 0xA9
	{"XOR E", 0, [&](){ xor_rb(&REG.E);}},          // 0xAA
	{"XOR H", 0, [&](){ xor_rb(&REG.H);}},          // 0xAC
	{"XOR L", 0, [&](){ xor_rb(&REG.L);}},          // 0xAD
	{"XOR (HL)", 0, [&](){ xor_atHL();}},       // 0xAE
	{"XOR A", 0, [&](){ xor_rb(&REG.A);}},          // 0xAF

	{"OR B", 0, [&](){ or_rb(&REG.B); }},           // 0xB0
	{"OR C", 0, [&](){ or_rb(&REG.C); }},           // 0xB1
	{"OR D", 0, [&](){ or_rb(&REG.D); }},           // 0xB2
	{"OR E", 0, [&](){ or_rb(&REG.E); }},           // 0xB3
	{"OR H", 0, [&](){ or_rb(&REG.H); }},           // 0xB4
	{"OR L", 0, [&](){ or_rb(&REG.L); }},           // 0xB5
	{"OR (HL)", 0, [&](){ or_atHL(); }},        // 0xB6
	{"OR A", 0, [&](){ or_rb(&REG.A); }},           // 0xB7
	{"CP B", 0, [&](){ cp_rb(&REG.B); }},           // 0xB7
	{"CP C", 0, [&](){ cp_rb(&REG.C); }},           // 0xB8
	{"CP D", 0, [&](){ cp_rb(&REG.D); }},           // 0xB9
	{"CP E", 0, [&](){ cp_rb(&REG.E); }},           // 0xBA
	{"CP H", 0, [&](){ cp_rb(&REG.H); }},           // 0xBC
	{"CP L", 0, [&](){ cp_rb(&REG.L); }},           // 0xBD
	{"CP (HL)", 0, [&](){ cp_atHL(); }},        // 0xBE
	{"CP A", 0, [&](){ cp_rb(&REG.A); }},           // 0xBF

	{"RET NZ", 0, TODO},         // 0xC0
	{"POP BC", 0, TODO},         // 0xC1
	{"JP NZ, 0x%04X", 2, TODO},  // 0xC2
	{"JP 0x%04X", 2, TODO},      // 0xC3
	{"CALL NZ, 0x%04X", 2, TODO},// 0xC4
	{"PUSH BC", 0, TODO},        // 0xC5
	{"ADD A, 0x%02X", 1, TODO},  // 0xC6
	{"RST 0", 0, TODO},          // 0xC7
	{"RET Z", 0, TODO},          // 0xC7
	{"RET", 0, TODO},            // 0xC8
	{"JP Z, 0x%04X", 2, TODO},   // 0xC9
	{"Ext Op", 0, TODO},         // 0xCA
	{"CALL Z, 0x%04X", 2, TODO}, // 0xCC
	{"CALL 0x%04X", 2, TODO},    // 0xCD
	{"ADC A, 0x%02X", 1, TODO},  // 0xCE
	{"RST 8", 0, TODO},          // 0xCF

	{"RET NC", 0, TODO},         // 0xD0
	{"POP DE", 0, TODO},         // 0xD1
	{"JP NC, 0x%04X", 2, TODO},  // 0xD2
	{"XX", 0, TODO},      // 0xD3
	{"CALL NC, 0x%04X", 2, TODO},// 0xD4
	{"PUSH DE", 0, TODO},        // 0xD5
	{"SUB A, 0x%02X", 1, TODO},  // 0xD6
	{"RST 10", 0, TODO},          // 0xD7
	{"RET C", 0, TODO},          // 0xD7
	{"RETI", 0, TODO},            // 0xD8
	{"JP C, 0x%04X", 2, TODO},   // 0xD9
	{"XX", 0, TODO},         // 0xDA
	{"CALL C, 0x%04X", 2, TODO}, // 0xDC
	{"XX", 0, TODO},    // 0xDD
	{"SBC A, 0x%02X", 1, TODO},  // 0xDE
	{"RST 18", 0, TODO},          // 0xDF

	{"LDH (0x%02X), A", 1, TODO},         // 0xE0
	{"POP HL", 0, TODO},         // 0xE1
	{"LDH (C), A", 0, TODO},  // 0xE2
	{"XX", 0, TODO},      // 0xE3
	{"XX", 0, TODO},// 0xE4
	{"PUSH HL", 0, TODO},        // 0xE5
	{"AND n", 1, [&](){ and_n(); }},  // 0xE6
	{"RST 20", 0, TODO},          // 0xE7
	{"ADD SP, d", 0, TODO},          // 0xE7
	{"JP (HL)", 0, TODO},            // 0xE8
	{"LD (0x%04X), A", 2, TODO},   // 0xE9
	{"XX", 0, TODO},         // 0xEA
	{"XX", 0, TODO}, // 0xEC
	{"XX", 0, TODO},    // 0xED
	{"XOR n", 1, [&](){ xor_n(); }},  // 0xEE
	{"RST 28", 0, TODO},          // 0xEF

	{"LDH A, (0x%02X)", 1, TODO},         // 0xF0
	{"POP AF", 0, TODO},         // 0xF1
	{"XX", 0, TODO},  // 0xF2
	{"DI", 0, TODO},      // 0xF3
	{"XX", 0, TODO},// 0xF4
	{"PUSH AF", 0, TODO},        // 0xF5
	{"OR n", 1, [&](){ or_n(); }},  // 0xF6
	{"RST 30", 0, TODO},          // 0xF7
	{"LDHL SP, d", 0, TODO},          // 0xF7
	{"LD SP, HL", 0, [&](){ ld_SP_HL(); }},            // 0xF8
	{"LD A, (0x%04X)", 2, TODO},   // 0xF9
	{"EI", 0, TODO},         // 0xFA
	{"XX", 0, TODO}, // 0xFC
	{"XX", 0, TODO},    // 0xFD
	{"CP n", 1, [&](){ cp_n(); }},  // 0xFE
	{"RST 38", 0, TODO}          // 0xFF

};