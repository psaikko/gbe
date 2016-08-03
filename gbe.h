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

const uint8_t FLAG_C = 0x10;
const uint8_t FLAG_H = 0x20;
const uint8_t FLAG_N = 0x40;
const uint8_t FLAG_Z = 0x80;

const uint8_t BIT_0 = 0x01;
const uint8_t BIT_1 = 0x02;
const uint8_t BIT_2 = 0x04;
const uint8_t BIT_3 = 0x08;
const uint8_t BIT_4 = 0x10;
const uint8_t BIT_5 = 0x20;
const uint8_t BIT_6 = 0x40;
const uint8_t BIT_7 = 0x80;

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

bool get_flag(uint8_t flag) {
	return REG.F & flag;
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
		// switch by 8192 byte segments
		switch(addr >> 15) {
			case 0:
				if (bios) {
					if (REG.PC < 100) 
						return &BIOS[addr];
					else
						bios = false;
				}
			case 1:
				return &ROM0[addr];
			case 2:
			case 3:
				return &ROM1[addr & 0x3FFF];
			case 4:
				return &grRAM[addr & 0x1FFF];
			case 5:
				return &extRAM[addr & 0x1FFF];
			case 6:
				return &RAM[addr & 0x1FFF];
			case 7:
				switch (addr & 0xFFF8) {
					case 0xFE00:
					case 0xFE80:
						if (addr < 0xFEA0)
							return &SPR[addr & 0x000F];
						else 
							return 0; // TODO 
					case 0xFF00:
						return &IO[addr & 0x000E];
					case 0xFF80:
						return &ZERO[addr & 0x000E];
					default:
						return &RAM[addr & 0x1FFF];
				}
		}
	}

	uint8_t readByte(uint16_t addr) {
		return BIOS[addr];
	}

	uint16_t readWord(uint16_t addr) {
		return *reinterpret_cast<uint16_t*>(getPtr(addr)); 
	}

	void writeByte(uint16_t addr, uint8_t val) {
		*getPtr(addr) = val;
	}

	void writeWord(uint16_t addr, uint16_t val) {
		uint16_t *wptr = reinterpret_cast<uint16_t*>(getPtr(addr)); 
		*wptr = val;
	}
} memory;

memory MEM;

#define ARGBYTE (MEM.readByte(REG.PC + 1))
#define ARGWORD (MEM.readWord(REG.PC + 1))

void nop() {
	REG.T += 4;
	REG.PC += 1;
}

// 8-bit arithmetic

void inc_rb(uint8_t * ptr) {
	(*ptr)++;

	unset_flag(FLAG_N);
	set_flag_cond(FLAG_H, (*ptr) & 0x0F == 0);
	set_flag_cond(FLAG_Z, (*ptr) == 0);
	REG.T += 4;
	REG.PC += 1;
}

void inc_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	MEM.writeByte(REG.HL, val + 1);

	unset_flag(FLAG_N);
	set_flag_cond(FLAG_H, val & 0x0F == 0x0F);
	set_flag_cond(FLAG_Z, (val + 1) == 0);

	REG.T += 12;
	REG.PC += 1;
}

void xor_rb(uint8_t * from) {
	REG.A ^= *from;
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 4;
	REG.PC += 1;
} 

void xor_n() {
	REG.A ^= ARGBYTE;
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
	REG.PC += 2;
}

void xor_atHL() {
	uint16_t addr = MEM.readWord(REG.HL); 
	REG.A ^= MEM.readByte(addr);
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
	REG.PC += 1;
}

void or_rb(uint8_t * from) {
	REG.A |= *from;
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 4;
	REG.PC += 1;
} 

void or_n() {
	REG.A |= ARGBYTE;
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
	REG.PC += 2;
}

void or_atHL() {
	uint16_t addr = MEM.readWord(REG.HL);
	REG.A |= MEM.readByte(addr);
	
	unset_flag(FLAG_H | FLAG_N | FLAG_C);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
	REG.PC += 1;
}

void and_rb(uint8_t * from) {
	REG.A &= *from;
	
	unset_flag(FLAG_N | FLAG_C);
	set_flag(FLAG_H);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 4;
	REG.PC += 1;
} 

void and_n() {
	REG.A &= ARGBYTE;
	
	unset_flag(FLAG_N | FLAG_C);
	set_flag(FLAG_H);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
	REG.PC += 2;
}

void and_atHL() {
	uint16_t addr = MEM.readWord(REG.HL);
	REG.A &= MEM.readByte(addr);
	
	unset_flag(FLAG_N | FLAG_C);
	set_flag(FLAG_H);
	if (REG.A == 0) set_flag(FLAG_Z);
	else unset_flag(FLAG_Z);

	REG.T += 8;
	REG.PC += 1;
}

void cp_rb(uint8_t * from) {

	uint8_t e = *from;

	set_flag(FLAG_N);
	set_flag_cond(FLAG_Z, e == REG.A);
	set_flag_cond(FLAG_C, e > REG.A);
	set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

	REG.T += 4;
	REG.PC += 1;
} 

void cp_n() {

	uint8_t e = ARGBYTE;

	set_flag(FLAG_N);
	set_flag_cond(FLAG_Z, e == REG.A);
	set_flag_cond(FLAG_C, e > REG.A);
	set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

	REG.T += 8;
	REG.PC += 2;
}

void cp_atHL() {

	uint8_t e = MEM.readByte(MEM.readWord(REG.HL));
	
	set_flag(FLAG_N);
	set_flag_cond(FLAG_Z, e == REG.A);
	set_flag_cond(FLAG_C, e > REG.A);
	set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

	REG.T += 8;
	REG.PC += 1;
}

// 16-bit arithmetic

void inc_rw(uint16_t * ptr) {
	(*ptr)++;
	REG.T += 8;	
	REG.PC += 1;
}

// 8-bit loads

void ld_rb_rb(uint8_t *to, uint8_t *from) {
	(*to) = (*from);
	REG.T += 4;	
	REG.PC += 1;
}

void ld_rb_n(uint8_t *to) {
	(*to) = ARGBYTE;
	REG.T += 8;	
	REG.PC += 2;
}

void ld_rb_atHL(uint8_t *to) {
	(*to) = MEM.readByte(REG.HL);
	REG.T += 12;	
	REG.PC += 1;	
}

void ld_atHL_rb(uint8_t *from) {
	MEM.writeByte(REG.HL, *from);
	REG.T += 8;
	REG.PC += 1;
}

void ld_atHL_n() {
	MEM.writeByte(REG.HL, ARGBYTE);
	REG.T += 12;
	REG.PC += 2;
}

void ld_A_atrw(uint16_t *addr) {
	REG.A = MEM.readWord(*addr);
	REG.T += 8;
	REG.PC += 1;
}

void ld_A_atnn() {
	REG.A = MEM.readByte(ARGWORD);
	REG.T += 16;
	REG.PC += 3;
}

void ld_atrw_A(uint16_t *addr) {
	MEM.writeByte(*addr, REG.A);
	REG.T += 8;
	REG.PC += 1;
}

void ld_atnn_A() {
	MEM.writeByte(ARGWORD, REG.A);
	REG.T += 16;
	REG.PC += 3;
}

void ldh_A_atC() {
	REG.A = MEM.readByte(0xFF00 | REG.C);
	REG.T += 8;
	REG.PC += 1;
}

void ldh_atC_A() {
	MEM.writeByte(0xFF00| REG.C, REG.A);
	REG.T += 8;
	REG.PC += 1;
}

void ldd_A_atHL() {
	REG.A = MEM.readByte(REG.HL);
	REG.HL--;
	REG.T += 8;
	REG.PC += 1;
}

void ldd_atHL_A() {
	MEM.writeByte(REG.HL, REG.A);
	REG.HL--;
	REG.T += 8;
	REG.PC += 1;
}

void ldi_A_atHL() {
	REG.A = MEM.readByte(REG.HL);
	REG.HL++;
	REG.T += 8;
	REG.PC += 1;
}

void ldi_atHL_A() {
	MEM.writeByte(REG.HL, REG.A);
	REG.HL++;
	REG.T += 8;
	REG.PC += 1;
}

void ldh_atn_A() {
	MEM.writeByte(0xFF00 | ARGBYTE, REG.A);
	REG.T += 12;
	REG.PC += 2;
}

void ldh_A_atn() {
	REG.A = MEM.readByte(0xFF00 | ARGBYTE);
	REG.T += 12;
	REG.PC += 2;
}

// 16-bit loads

void ld_rw_nn(uint16_t* to) {
	*to = ARGWORD;
	REG.T += 12;
	REG.PC += 3;
}

void ld_atnn_SP() {
	uint16_t addr = ARGWORD;
	MEM.writeWord(addr, REG.SP);
	REG.T += 20;
	REG.PC += 3;
}

void ld_SP_HL() {
	REG.SP = REG.HL;
	REG.T += 8;
	REG.PC += 1;
}

// ext block

// rotate right
void rrc_rb(uint8_t * at) {
	bool carry = *at & BIT_0;
	set_flag_cond(FLAG_C, carry);
	*at >>= 1;
	if (carry) *at |= BIT_7;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void rrc_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	bool carry = val & BIT_0;
	set_flag_cond(FLAG_C, carry);
	val >>= 1;
	if (carry) val |= BIT_7;
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;
	REG.PC += 1;
}

// rotate left
void rlc_rb(uint8_t * at) {
	bool carry = *at & BIT_7;
	set_flag_cond(FLAG_C, carry);
	*at <<= 1;
	if (carry) *at |= BIT_0;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void rlc_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	bool carry = val & BIT_7;
	set_flag_cond(FLAG_C, carry);
	val <<= 1;
	if (carry) val |= BIT_0;
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;
	REG.PC += 1;
}

// rotate right (through carry flag)
void rr_rb(uint8_t * at) {
	bool carry = get_flag(FLAG_C);
	set_flag_cond(FLAG_C, *at & BIT_0);
	*at >>= 1;
	if (carry) *at |= BIT_7;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void rr_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	bool carry = get_flag(FLAG_C);
	set_flag_cond(FLAG_C, val & BIT_0);
	val >>= 1;
	if (carry) val |= BIT_7;
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;
	REG.PC += 1;
}

// rotate left (through carry flag)
void rl_rb(uint8_t * at) {
	bool carry = get_flag(FLAG_C);
	set_flag_cond(FLAG_C, *at & BIT_7);
	*at <<= 1;
	if (carry) *at |= BIT_0;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void rl_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	bool carry = get_flag(FLAG_C);
	set_flag_cond(FLAG_C, val & BIT_7);
	val <<= 1;
	if (carry) val |= BIT_0;
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;
	REG.PC += 1;
}

// shift left
void sla_rb(uint8_t * at) {
	set_flag_cond(FLAG_C, *at & BIT_7);
	*at <<= 1;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void sla_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	set_flag_cond(FLAG_C, val & BIT_7);
	val <<= 1;
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;
	REG.PC += 1;
}

// shift right (keep bit 7)
void sra_rb(uint8_t * at) {
	set_flag_cond(FLAG_C, *at & BIT_0);
	*at >>= 1;
	if (*at & BIT_6) *at |= BIT_7;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void sra_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	set_flag_cond(FLAG_C, val & BIT_0);
	val >>= 1;
	if (val & BIT_6) val |= BIT_7;
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;
	REG.PC += 1;
}

// shift right
void srl_rb(uint8_t * at) {
	set_flag_cond(FLAG_C, *at & BIT_0);
	*at >>= 1;

	unset_flag(FLAG_N | FLAG_H);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void srl_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	set_flag_cond(FLAG_C, val & BIT_0);
	val >>= 1;
	MEM.writeByte(REG.HL, val);

	REG.T += 8;	
	REG.PC += 1;
}

void swap_rb(uint8_t * at) {
	uint8_t tmp = *at & 0x0F;
	*at >>= 4;
	*at &= (tmp << 4);

	unset_flag(FLAG_N | FLAG_H | FLAG_C);
	set_flag_cond(FLAG_Z, *at == 0);

	REG.T += 8;
	REG.PC += 1;
}

void swap_atHL() {
	uint8_t val = MEM.readByte(REG.HL);
	uint8_t tmp = val & 0x0F;
	val >>= 4;
	val &= (tmp << 4);
	MEM.writeByte(REG.HL, val);

	unset_flag(FLAG_N | FLAG_H | FLAG_C);
	set_flag_cond(FLAG_Z, val == 0);

	REG.T += 16;	
	REG.PC += 1;
}

void bit_i_rb(const uint8_t mask, uint8_t *from) {
	unset_flag(FLAG_N);
	unset_flag(FLAG_H);
	set_flag_cond(FLAG_Z, (*from & mask) == 0);
	REG.T += 8;
	REG.PC += 1;
}

void bit_i_atHL(const uint8_t mask) {
	unset_flag(FLAG_N);
	unset_flag(FLAG_H);
	set_flag_cond(FLAG_Z, (MEM.readByte(REG.HL) & mask) == 0);
	REG.T += 12;
	REG.PC += 1;
}

void res_i_rb(const uint8_t mask, uint8_t * to) {
	*to &= ~mask;
	REG.T += 8;
	REG.PC += 1;
}

void res_i_atHL(const uint8_t mask) {
	uint8_t val = MEM.readByte(REG.HL);
	val &= ~mask;
	MEM.writeByte(REG.HL, val);

	REG.T += 16;
	REG.PC += 1;
}

void set_i_rb(const uint8_t mask, uint8_t * to) {
	*to |= mask;
	REG.T += 8;
	REG.PC += 1;
}

void set_i_atHL(const uint8_t mask) {
	uint8_t val = MEM.readByte(REG.HL);
	val |= mask;
	MEM.writeByte(REG.HL, val);

	REG.T += 16;
	REG.PC += 1;
}

// jumps

void jp_nn() {
	uint16_t target = ARGWORD;
}

void jp_f_nn(uint8_t mask) {
	uint16_t target = ARGWORD;

}

void jp_atHL() {

}

void jr_e() {
	uint8_t n = MEM.readByte(REG.PC + 1);
	int8_t* e = reinterpret_cast<int8_t*>(&n);
}

void jr_f_e(uint8_t mask) {
	uint8_t n = MEM.readByte(REG.PC + 1);
	int8_t* e = reinterpret_cast<int8_t*>(&n);
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

instruction ext_instructions[256] = {
	{"RLC B", 0, [&](){ rlc_rb(&REG.B); }}, //0xCB01
	{"RLC C", 0, [&](){ rlc_rb(&REG.C);}}, //0xCB02
	{"RLC D", 0, [&](){ rlc_rb(&REG.D);}}, //0xCB03
	{"RLC E", 0, [&](){ rlc_rb(&REG.E);}}, //0xCB04
	{"RLC H", 0, [&](){ rlc_rb(&REG.H);}}, //0xCB05
	{"RLC L", 0, [&](){ rlc_rb(&REG.L);}}, //0xCB06
	{"RLC (HL)", 0, [&](){ rlc_atHL();}}, //0xCB07
	{"RLC A", 0, [&](){ rlc_rb(&REG.A);}}, //0xCB08
	{"RRC B", 0, [&](){ rrc_rb(&REG.B);}}, //0xCB09
	{"RRC C", 0, [&](){ rrc_rb(&REG.C);}}, //0xCB0A
	{"RRC D", 0, [&](){ rrc_rb(&REG.D);}}, //0xCB0B
	{"RRC E", 0, [&](){ rrc_rb(&REG.E);}}, //0xCB0C
	{"RRC H", 0, [&](){ rrc_rb(&REG.H);}}, //0xCB0D
	{"RRC L", 0, [&](){ rrc_rb(&REG.L);}}, //0xCB0E 
	{"RRC (HL)", 0, [&](){ rrc_atHL();}}, //0xCB0F
	{"RRC A", 0, [&](){ rrc_rb(&REG.A);}}, //0xCB01

	{"RL B", 0, [&](){ rl_rb(&REG.B); }}, //0xCB11
	{"RL C", 0, [&](){ rl_rb(&REG.C);}}, //0xCB12
	{"RL D", 0, [&](){ rl_rb(&REG.D);}}, //0xCB13
	{"RL E", 0, [&](){ rl_rb(&REG.E);}}, //0xCB14
	{"RL H", 0, [&](){ rl_rb(&REG.H);}}, //0xCB15
	{"RL L", 0, [&](){ rl_rb(&REG.L);}}, //0xCB16
	{"RL (HL)", 0, [&](){ rl_atHL();}}, //0xCB17
	{"RL A", 0, [&](){ rl_rb(&REG.A);}}, //0xCB18
	{"RR B", 0, [&](){ rr_rb(&REG.B);}}, //0xCB19
	{"RR C", 0, [&](){ rr_rb(&REG.C);}}, //0xCB1A
	{"RR D", 0, [&](){ rr_rb(&REG.D);}}, //0xCB1B
	{"RR E", 0, [&](){ rr_rb(&REG.E);}}, //0xCB1C
	{"RR H", 0, [&](){ rr_rb(&REG.H);}}, //0xCB1D
	{"RR L", 0, [&](){ rr_rb(&REG.L);}}, //0xCB1E 
	{"RR (HL)", 0, [&](){ rr_atHL();}}, //0xCB1F
	{"RR A", 0, [&](){ rr_rb(&REG.A);}}, //0xCB11

	{"SLA B", 0, [&](){ sla_rb(&REG.B); }}, //0xCB21
	{"SLA C", 0, [&](){ sla_rb(&REG.C);}}, //0xCB22
	{"SLA D", 0, [&](){ sla_rb(&REG.D);}}, //0xCB23
	{"SLA E", 0, [&](){ sla_rb(&REG.E);}}, //0xCB24
	{"SLA H", 0, [&](){ sla_rb(&REG.H);}}, //0xCB25
	{"SLA L", 0, [&](){ sla_rb(&REG.L);}}, //0xCB26
	{"SLA (HL)", 0, [&](){ sla_atHL();}}, //0xCB27
	{"SLA A", 0, [&](){ sla_rb(&REG.A);}}, //0xCB28
	{"SRA B", 0, [&](){ sra_rb(&REG.B);}}, //0xCB29
	{"SRA C", 0, [&](){ sra_rb(&REG.C);}}, //0xCB2A
	{"SRA D", 0, [&](){ sra_rb(&REG.D);}}, //0xCB2B
	{"SRA E", 0, [&](){ sra_rb(&REG.E);}}, //0xCB2C
	{"SRA H", 0, [&](){ sra_rb(&REG.H);}}, //0xCB2D
	{"SRA L", 0, [&](){ sra_rb(&REG.L);}}, //0xCB2E 
	{"SRA (HL)", 0, [&](){ sra_atHL();}}, //0xCB2F
	{"SRA A", 0, [&](){ sra_rb(&REG.A);}}, //0xCB21

	{"SWAP B", 0, [&](){ swap_rb(&REG.B); }}, //0xCB31
	{"SWAP C", 0, [&](){ swap_rb(&REG.C);}}, //0xCB32
	{"SWAP D", 0, [&](){ swap_rb(&REG.D);}}, //0xCB33
	{"SWAP E", 0, [&](){ swap_rb(&REG.E);}}, //0xCB34
	{"SWAP H", 0, [&](){ swap_rb(&REG.H);}}, //0xCB35
	{"SWAP L", 0, [&](){ swap_rb(&REG.L);}}, //0xCB36
	{"SWAP (HL)", 0, [&](){ swap_atHL();}}, //0xCB37
	{"SWAP A", 0, [&](){ swap_rb(&REG.A);}}, //0xCB38
	{"SRL B", 0, [&](){ srl_rb(&REG.B);}}, //0xCB39
	{"SRL C", 0, [&](){ srl_rb(&REG.C);}}, //0xCB3A
	{"SRL D", 0, [&](){ srl_rb(&REG.D);}}, //0xCB3B
	{"SRL E", 0, [&](){ srl_rb(&REG.E);}}, //0xCB3C
	{"SRL H", 0, [&](){ srl_rb(&REG.H);}}, //0xCB3D
	{"SRL L", 0, [&](){ srl_rb(&REG.L);}}, //0xCB3E 
	{"SRL (HL)", 0, [&](){ srl_atHL();}}, //0xCB3F
	{"SRL A", 0, [&](){ srl_rb(&REG.A);}}, //0xCB31

	{"BIT 0, B", 0, [&](){ bit_i_rb(BIT_0, &REG.B); }}, //0xCB41
	{"BIT 0, C", 0, [&](){ bit_i_rb(BIT_0, &REG.C);}}, //0xCB42
	{"BIT 0, D", 0, [&](){ bit_i_rb(BIT_0, &REG.D);}}, //0xCB43
	{"BIT 0, E", 0, [&](){ bit_i_rb(BIT_0, &REG.E);}}, //0xCB44
	{"BIT 0, H", 0, [&](){ bit_i_rb(BIT_0, &REG.H);}}, //0xCB45
	{"BIT 0, L", 0, [&](){ bit_i_rb(BIT_0, &REG.L);}}, //0xCB46
	{"BIT 0, (HL)", 0, [&](){ bit_i_atHL(BIT_0);}}, //0xCB47
	{"BIT 0, A", 0, [&](){ bit_i_rb(BIT_0, &REG.A);}}, //0xCB48
	{"BIT 1, B", 0, [&](){ bit_i_rb(BIT_1, &REG.B);}}, //0xCB49
	{"BIT 1, C", 0, [&](){ bit_i_rb(BIT_1, &REG.C);}}, //0xCB4A
	{"BIT 1, D", 0, [&](){ bit_i_rb(BIT_1, &REG.D);}}, //0xCB4B
	{"BIT 1, E", 0, [&](){ bit_i_rb(BIT_1, &REG.E);}}, //0xCB4C
	{"BIT 1, H", 0, [&](){ bit_i_rb(BIT_1, &REG.H);}}, //0xCB4D
	{"BIT 1, L", 0, [&](){ bit_i_rb(BIT_1, &REG.L);}}, //0xCB4E 
	{"BIT 1, (HL)", 0, [&](){ bit_i_atHL(BIT_1);}}, //0xCB4F
	{"BIT 1, A", 0, [&](){ bit_i_rb(BIT_1, &REG.A);}}, //0xCB41
	{"BIT 2, B", 0, [&](){ bit_i_rb(BIT_2, &REG.B); }}, //0xCB51
	{"BIT 2, C", 0, [&](){ bit_i_rb(BIT_2, &REG.C);}}, //0xCB52
	{"BIT 2, D", 0, [&](){ bit_i_rb(BIT_2, &REG.D);}}, //0xCB53
	{"BIT 2, E", 0, [&](){ bit_i_rb(BIT_2, &REG.E);}}, //0xCB54
	{"BIT 2, H", 0, [&](){ bit_i_rb(BIT_2, &REG.H);}}, //0xCB55
	{"BIT 2, L", 0, [&](){ bit_i_rb(BIT_2, &REG.L);}}, //0xCB56
	{"BIT 2, (HL)", 0, [&](){ bit_i_atHL(BIT_2);}}, //0xCB57
	{"BIT 2, A", 0, [&](){ bit_i_rb(BIT_2, &REG.A);}}, //0xCB58
	{"BIT 3, B", 0, [&](){ bit_i_rb(BIT_3, &REG.B);}}, //0xCB59
	{"BIT 3, C", 0, [&](){ bit_i_rb(BIT_3, &REG.C);}}, //0xCB5A
	{"BIT 3, D", 0, [&](){ bit_i_rb(BIT_3, &REG.D);}}, //0xCB5B
	{"BIT 3, E", 0, [&](){ bit_i_rb(BIT_3, &REG.E);}}, //0xCB5C
	{"BIT 3, H", 0, [&](){ bit_i_rb(BIT_3, &REG.H);}}, //0xCB5D
	{"BIT 3, L", 0, [&](){ bit_i_rb(BIT_3, &REG.L);}}, //0xCB5E 
	{"BIT 3, (HL)", 0, [&](){ bit_i_atHL(BIT_3);}}, //0xCB5F
	{"BIT 3, A", 0, [&](){ bit_i_rb(BIT_3, &REG.A);}}, //0xCB51
	{"BIT 4, B", 0, [&](){ bit_i_rb(BIT_4, &REG.B); }}, //0xCB61
	{"BIT 4, C", 0, [&](){ bit_i_rb(BIT_4, &REG.C);}}, //0xCB62
	{"BIT 4, D", 0, [&](){ bit_i_rb(BIT_4, &REG.D);}}, //0xCB63
	{"BIT 4, E", 0, [&](){ bit_i_rb(BIT_4, &REG.E);}}, //0xCB64
	{"BIT 4, H", 0, [&](){ bit_i_rb(BIT_4, &REG.H);}}, //0xCB65
	{"BIT 4, L", 0, [&](){ bit_i_rb(BIT_4, &REG.L);}}, //0xCB66
	{"BIT 4, (HL)", 0, [&](){ bit_i_atHL(BIT_4);}}, //0xCB67
	{"BIT 4, A", 0, [&](){ bit_i_rb(BIT_4, &REG.A);}}, //0xCB68
	{"BIT 5, B", 0, [&](){ bit_i_rb(BIT_5, &REG.B);}}, //0xCB69
	{"BIT 5, C", 0, [&](){ bit_i_rb(BIT_5, &REG.C);}}, //0xCB6A
	{"BIT 5, D", 0, [&](){ bit_i_rb(BIT_5, &REG.D);}}, //0xCB6B
	{"BIT 5, E", 0, [&](){ bit_i_rb(BIT_5, &REG.E);}}, //0xCB6C
	{"BIT 5, H", 0, [&](){ bit_i_rb(BIT_5, &REG.H);}}, //0xCB6D
	{"BIT 5, L", 0, [&](){ bit_i_rb(BIT_5, &REG.L);}}, //0xCB6E 
	{"BIT 5, (HL)", 0, [&](){ bit_i_atHL(BIT_5);}}, //0xCB6F
	{"BIT 5, A", 0, [&](){ bit_i_rb(BIT_5, &REG.A);}}, //0xCB61
	{"BIT 6, B", 0, [&](){ bit_i_rb(BIT_6, &REG.B); }}, //0xCB71
	{"BIT 6, C", 0, [&](){ bit_i_rb(BIT_6, &REG.C);}}, //0xCB72
	{"BIT 6, D", 0, [&](){ bit_i_rb(BIT_6, &REG.D);}}, //0xCB73
	{"BIT 6, E", 0, [&](){ bit_i_rb(BIT_6, &REG.E);}}, //0xCB74
	{"BIT 6, H", 0, [&](){ bit_i_rb(BIT_6, &REG.H);}}, //0xCB75
	{"BIT 6, L", 0, [&](){ bit_i_rb(BIT_6, &REG.L);}}, //0xCB76
	{"BIT 6, (HL)", 0, [&](){ bit_i_atHL(BIT_6);}}, //0xCB77
	{"BIT 6, A", 0, [&](){ bit_i_rb(BIT_6, &REG.A);}}, //0xCB78
	{"BIT 7, B", 0, [&](){ bit_i_rb(BIT_7, &REG.B);}}, //0xCB79
	{"BIT 7, C", 0, [&](){ bit_i_rb(BIT_7, &REG.C);}}, //0xCB7A
	{"BIT 7, D", 0, [&](){ bit_i_rb(BIT_7, &REG.D);}}, //0xCB7B
	{"BIT 7, E", 0, [&](){ bit_i_rb(BIT_7, &REG.E);}}, //0xCB7C
	{"BIT 7, H", 0, [&](){ bit_i_rb(BIT_7, &REG.H);}}, //0xCB7D
	{"BIT 7, L", 0, [&](){ bit_i_rb(BIT_7, &REG.L);}}, //0xCB7E 
	{"BIT 7, (HL)", 0, [&](){ bit_i_atHL(BIT_7);}}, //0xCB7F
	{"BIT 7, A", 0, [&](){ bit_i_rb(BIT_7, &REG.A);}}, //0xCB71	

	{"RES 0, B", 0, [&](){ res_i_rb(BIT_0, &REG.B); }}, //0xCB41
	{"RES 0, C", 0, [&](){ res_i_rb(BIT_0, &REG.C);}}, //0xCB42
	{"RES 0, D", 0, [&](){ res_i_rb(BIT_0, &REG.D);}}, //0xCB43
	{"RES 0, E", 0, [&](){ res_i_rb(BIT_0, &REG.E);}}, //0xCB44
	{"RES 0, H", 0, [&](){ res_i_rb(BIT_0, &REG.H);}}, //0xCB45
	{"RES 0, L", 0, [&](){ res_i_rb(BIT_0, &REG.L);}}, //0xCB46
	{"RES 0, (HL)", 0, [&](){ res_i_atHL(BIT_0);}}, //0xCB47
	{"RES 0, A", 0, [&](){ res_i_rb(BIT_0, &REG.A);}}, //0xCB48
	{"RES 1, B", 0, [&](){ res_i_rb(BIT_1, &REG.B);}}, //0xCB49
	{"RES 1, C", 0, [&](){ res_i_rb(BIT_1, &REG.C);}}, //0xCB4A
	{"RES 1, D", 0, [&](){ res_i_rb(BIT_1, &REG.D);}}, //0xCB4B
	{"RES 1, E", 0, [&](){ res_i_rb(BIT_1, &REG.E);}}, //0xCB4C
	{"RES 1, H", 0, [&](){ res_i_rb(BIT_1, &REG.H);}}, //0xCB4D
	{"RES 1, L", 0, [&](){ res_i_rb(BIT_1, &REG.L);}}, //0xCB4E 
	{"RES 1, (HL)", 0, [&](){ res_i_atHL(BIT_1);}}, //0xCB4F
	{"RES 1, A", 0, [&](){ res_i_rb(BIT_1, &REG.A);}}, //0xCB41
	{"RES 2, B", 0, [&](){ res_i_rb(BIT_2, &REG.B); }}, //0xCB51
	{"RES 2, C", 0, [&](){ res_i_rb(BIT_2, &REG.C);}}, //0xCB52
	{"RES 2, D", 0, [&](){ res_i_rb(BIT_2, &REG.D);}}, //0xCB53
	{"RES 2, E", 0, [&](){ res_i_rb(BIT_2, &REG.E);}}, //0xCB54
	{"RES 2, H", 0, [&](){ res_i_rb(BIT_2, &REG.H);}}, //0xCB55
	{"RES 2, L", 0, [&](){ res_i_rb(BIT_2, &REG.L);}}, //0xCB56
	{"RES 2, (HL)", 0, [&](){ res_i_atHL(BIT_2);}}, //0xCB57
	{"RES 2, A", 0, [&](){ res_i_rb(BIT_2, &REG.A);}}, //0xCB58
	{"RES 3, B", 0, [&](){ res_i_rb(BIT_3, &REG.B);}}, //0xCB59
	{"RES 3, C", 0, [&](){ res_i_rb(BIT_3, &REG.C);}}, //0xCB5A
	{"RES 3, D", 0, [&](){ res_i_rb(BIT_3, &REG.D);}}, //0xCB5B
	{"RES 3, E", 0, [&](){ res_i_rb(BIT_3, &REG.E);}}, //0xCB5C
	{"RES 3, H", 0, [&](){ res_i_rb(BIT_3, &REG.H);}}, //0xCB5D
	{"RES 3, L", 0, [&](){ res_i_rb(BIT_3, &REG.L);}}, //0xCB5E 
	{"RES 3, (HL)", 0, [&](){ res_i_atHL(BIT_3);}}, //0xCB5F
	{"RES 3, A", 0, [&](){ res_i_rb(BIT_3, &REG.A);}}, //0xCB51
	{"RES 4, B", 0, [&](){ res_i_rb(BIT_4, &REG.B); }}, //0xCB61
	{"RES 4, C", 0, [&](){ res_i_rb(BIT_4, &REG.C);}}, //0xCB62
	{"RES 4, D", 0, [&](){ res_i_rb(BIT_4, &REG.D);}}, //0xCB63
	{"RES 4, E", 0, [&](){ res_i_rb(BIT_4, &REG.E);}}, //0xCB64
	{"RES 4, H", 0, [&](){ res_i_rb(BIT_4, &REG.H);}}, //0xCB65
	{"RES 4, L", 0, [&](){ res_i_rb(BIT_4, &REG.L);}}, //0xCB66
	{"RES 4, (HL)", 0, [&](){ res_i_atHL(BIT_4);}}, //0xCB67
	{"RES 4, A", 0, [&](){ res_i_rb(BIT_4, &REG.A);}}, //0xCB68
	{"RES 5, B", 0, [&](){ res_i_rb(BIT_5, &REG.B);}}, //0xCB69
	{"RES 5, C", 0, [&](){ res_i_rb(BIT_5, &REG.C);}}, //0xCB6A
	{"RES 5, D", 0, [&](){ res_i_rb(BIT_5, &REG.D);}}, //0xCB6B
	{"RES 5, E", 0, [&](){ res_i_rb(BIT_5, &REG.E);}}, //0xCB6C
	{"RES 5, H", 0, [&](){ res_i_rb(BIT_5, &REG.H);}}, //0xCB6D
	{"RES 5, L", 0, [&](){ res_i_rb(BIT_5, &REG.L);}}, //0xCB6E 
	{"RES 5, (HL)", 0, [&](){ res_i_atHL(BIT_5);}}, //0xCB6F
	{"RES 5, A", 0, [&](){ res_i_rb(BIT_5, &REG.A);}}, //0xCB61
	{"RES 6, B", 0, [&](){ res_i_rb(BIT_6, &REG.B); }}, //0xCB71
	{"RES 6, C", 0, [&](){ res_i_rb(BIT_6, &REG.C);}}, //0xCB72
	{"RES 6, D", 0, [&](){ res_i_rb(BIT_6, &REG.D);}}, //0xCB73
	{"RES 6, E", 0, [&](){ res_i_rb(BIT_6, &REG.E);}}, //0xCB74
	{"RES 6, H", 0, [&](){ res_i_rb(BIT_6, &REG.H);}}, //0xCB75
	{"RES 6, L", 0, [&](){ res_i_rb(BIT_6, &REG.L);}}, //0xCB76
	{"RES 6, (HL)", 0, [&](){ res_i_atHL(BIT_6);}}, //0xCB77
	{"RES 6, A", 0, [&](){ res_i_rb(BIT_6, &REG.A);}}, //0xCB78
	{"RES 7, B", 0, [&](){ res_i_rb(BIT_7, &REG.B);}}, //0xCB79
	{"RES 7, C", 0, [&](){ res_i_rb(BIT_7, &REG.C);}}, //0xCB7A
	{"RES 7, D", 0, [&](){ res_i_rb(BIT_7, &REG.D);}}, //0xCB7B
	{"RES 7, E", 0, [&](){ res_i_rb(BIT_7, &REG.E);}}, //0xCB7C
	{"RES 7, H", 0, [&](){ res_i_rb(BIT_7, &REG.H);}}, //0xCB7D
	{"RES 7, L", 0, [&](){ res_i_rb(BIT_7, &REG.L);}}, //0xCB7E 
	{"RES 7, (HL)", 0, [&](){ res_i_atHL(BIT_7);}}, //0xCB7F
	{"RES 7, A", 0, [&](){ res_i_rb(BIT_7, &REG.A);}}, //0xCB71	

	{"SET 0, B", 0, [&](){ set_i_rb(BIT_0, &REG.B); }}, //0xCB41
	{"SET 0, C", 0, [&](){ set_i_rb(BIT_0, &REG.C);}}, //0xCB42
	{"SET 0, D", 0, [&](){ set_i_rb(BIT_0, &REG.D);}}, //0xCB43
	{"SET 0, E", 0, [&](){ set_i_rb(BIT_0, &REG.E);}}, //0xCB44
	{"SET 0, H", 0, [&](){ set_i_rb(BIT_0, &REG.H);}}, //0xCB45
	{"SET 0, L", 0, [&](){ set_i_rb(BIT_0, &REG.L);}}, //0xCB46
	{"SET 0, (HL)", 0, [&](){ set_i_atHL(BIT_0);}}, //0xCB47
	{"SET 0, A", 0, [&](){ set_i_rb(BIT_0, &REG.A);}}, //0xCB48
	{"SET 1, B", 0, [&](){ set_i_rb(BIT_1, &REG.B);}}, //0xCB49
	{"SET 1, C", 0, [&](){ set_i_rb(BIT_1, &REG.C);}}, //0xCB4A
	{"SET 1, D", 0, [&](){ set_i_rb(BIT_1, &REG.D);}}, //0xCB4B
	{"SET 1, E", 0, [&](){ set_i_rb(BIT_1, &REG.E);}}, //0xCB4C
	{"SET 1, H", 0, [&](){ set_i_rb(BIT_1, &REG.H);}}, //0xCB4D
	{"SET 1, L", 0, [&](){ set_i_rb(BIT_1, &REG.L);}}, //0xCB4E 
	{"SET 1, (HL)", 0, [&](){ set_i_atHL(BIT_1);}}, //0xCB4F
	{"SET 1, A", 0, [&](){ set_i_rb(BIT_1, &REG.A);}}, //0xCB41
	{"SET 2, B", 0, [&](){ set_i_rb(BIT_2, &REG.B); }}, //0xCB51
	{"SET 2, C", 0, [&](){ set_i_rb(BIT_2, &REG.C);}}, //0xCB52
	{"SET 2, D", 0, [&](){ set_i_rb(BIT_2, &REG.D);}}, //0xCB53
	{"SET 2, E", 0, [&](){ set_i_rb(BIT_2, &REG.E);}}, //0xCB54
	{"SET 2, H", 0, [&](){ set_i_rb(BIT_2, &REG.H);}}, //0xCB55
	{"SET 2, L", 0, [&](){ set_i_rb(BIT_2, &REG.L);}}, //0xCB56
	{"SET 2, (HL)", 0, [&](){ set_i_atHL(BIT_2);}}, //0xCB57
	{"SET 2, A", 0, [&](){ set_i_rb(BIT_2, &REG.A);}}, //0xCB58
	{"SET 3, B", 0, [&](){ set_i_rb(BIT_3, &REG.B);}}, //0xCB59
	{"SET 3, C", 0, [&](){ set_i_rb(BIT_3, &REG.C);}}, //0xCB5A
	{"SET 3, D", 0, [&](){ set_i_rb(BIT_3, &REG.D);}}, //0xCB5B
	{"SET 3, E", 0, [&](){ set_i_rb(BIT_3, &REG.E);}}, //0xCB5C
	{"SET 3, H", 0, [&](){ set_i_rb(BIT_3, &REG.H);}}, //0xCB5D
	{"SET 3, L", 0, [&](){ set_i_rb(BIT_3, &REG.L);}}, //0xCB5E 
	{"SET 3, (HL)", 0, [&](){ set_i_atHL(BIT_3);}}, //0xCB5F
	{"SET 3, A", 0, [&](){ set_i_rb(BIT_3, &REG.A);}}, //0xCB51
	{"SET 4, B", 0, [&](){ set_i_rb(BIT_4, &REG.B); }}, //0xCB61
	{"SET 4, C", 0, [&](){ set_i_rb(BIT_4, &REG.C);}}, //0xCB62
	{"SET 4, D", 0, [&](){ set_i_rb(BIT_4, &REG.D);}}, //0xCB63
	{"SET 4, E", 0, [&](){ set_i_rb(BIT_4, &REG.E);}}, //0xCB64
	{"SET 4, H", 0, [&](){ set_i_rb(BIT_4, &REG.H);}}, //0xCB65
	{"SET 4, L", 0, [&](){ set_i_rb(BIT_4, &REG.L);}}, //0xCB66
	{"SET 4, (HL)", 0, [&](){ set_i_atHL(BIT_4);}}, //0xCB67
	{"SET 4, A", 0, [&](){ set_i_rb(BIT_4, &REG.A);}}, //0xCB68
	{"SET 5, B", 0, [&](){ set_i_rb(BIT_5, &REG.B);}}, //0xCB69
	{"SET 5, C", 0, [&](){ set_i_rb(BIT_5, &REG.C);}}, //0xCB6A
	{"SET 5, D", 0, [&](){ set_i_rb(BIT_5, &REG.D);}}, //0xCB6B
	{"SET 5, E", 0, [&](){ set_i_rb(BIT_5, &REG.E);}}, //0xCB6C
	{"SET 5, H", 0, [&](){ set_i_rb(BIT_5, &REG.H);}}, //0xCB6D
	{"SET 5, L", 0, [&](){ set_i_rb(BIT_5, &REG.L);}}, //0xCB6E 
	{"SET 5, (HL)", 0, [&](){ set_i_atHL(BIT_5);}}, //0xCB6F
	{"SET 5, A", 0, [&](){ set_i_rb(BIT_5, &REG.A);}}, //0xCB61
	{"SET 6, B", 0, [&](){ set_i_rb(BIT_6, &REG.B); }}, //0xCB71
	{"SET 6, C", 0, [&](){ set_i_rb(BIT_6, &REG.C);}}, //0xCB72
	{"SET 6, D", 0, [&](){ set_i_rb(BIT_6, &REG.D);}}, //0xCB73
	{"SET 6, E", 0, [&](){ set_i_rb(BIT_6, &REG.E);}}, //0xCB74
	{"SET 6, H", 0, [&](){ set_i_rb(BIT_6, &REG.H);}}, //0xCB75
	{"SET 6, L", 0, [&](){ set_i_rb(BIT_6, &REG.L);}}, //0xCB76
	{"SET 6, (HL)", 0, [&](){ set_i_atHL(BIT_6);}}, //0xCB77
	{"SET 6, A", 0, [&](){ set_i_rb(BIT_6, &REG.A);}}, //0xCB78
	{"SET 7, B", 0, [&](){ set_i_rb(BIT_7, &REG.B);}}, //0xCB79
	{"SET 7, C", 0, [&](){ set_i_rb(BIT_7, &REG.C);}}, //0xCB7A
	{"SET 7, D", 0, [&](){ set_i_rb(BIT_7, &REG.D);}}, //0xCB7B
	{"SET 7, E", 0, [&](){ set_i_rb(BIT_7, &REG.E);}}, //0xCB7C
	{"SET 7, H", 0, [&](){ set_i_rb(BIT_7, &REG.H);}}, //0xCB7D
	{"SET 7, L", 0, [&](){ set_i_rb(BIT_7, &REG.L);}}, //0xCB7E 
	{"SET 7, (HL)", 0, [&](){ set_i_atHL(BIT_7);}}, //0xCB7F
	{"SET 7, A", 0, [&](){ set_i_rb(BIT_7, &REG.A);}} //0xCB71	
};

void ext() {
	REG.PC += 1;
	ext_instructions[ARGBYTE].fn();
}

instruction instructions[256] = {
	{"NOP", 0, [&](){ nop(); }},            // 0x00
	{"LD BC, 0x%04X", 2, [&](){ ld_rw_nn(&REG.BC); }},  // 0x01
	{"LD (BC), A", 0, [&](){ ld_atrw_A(&REG.BC); }},     // 0x02
	{"INC BC", 0, [&](){ inc_rw(&REG.BC); }},         // 0x03
	{"INC B", 0, [&](){ inc_rb(&REG.B); }},          // 0x04
	{"DEC B", 0, TODO},          // 0x05
	{"LD B, 0x%02X", 1, [&](){ ld_rb_n(&REG.B); }},   // 0x06
	{"RLC A", 0, TODO},          // 0x07
	{"LD (0x%04X), SP", 2, [&](){ ld_atnn_SP(); }},// 0x08
	{"ADD HL, BC", 0, TODO},     // 0x09
	{"LD A, (BC)", 0, [&](){ ld_A_atrw(&REG.BC); }},     // 0x0A
	{"DEC BC", 0, TODO},         // 0x0B
	{"INC C", 0, [&](){ inc_rb(&REG.C); }},          // 0x0C
	{"DEC C", 0, TODO},          // 0x0D
	{"LD C, 0x%02X", 1, [&](){ ld_rb_n(&REG.C); }},   // 0x0E
	{"RRC A", 0, TODO},          // 0x0F

	{"STOP", 0, TODO},           // 0x10
	{"LD DE, 0x%04X", 2, [&](){ ld_rw_nn(&REG.DE); }},  // 0x11
	{"LD (DE), A", 0, [&](){ ld_atrw_A(&REG.DE); }},     // 0x12
	{"INC DE", 0, [&](){ inc_rw(&REG.DE); } },         // 0x13
	{"INC D", 0, [&](){ inc_rb(&REG.D); }},          // 0x14
	{"DEC D", 0, TODO},          // 0x15
	{"LD D, 0x%02X", 1, [&](){ ld_rb_n(&REG.D); }},   // 0x16
	{"RL A", 0, TODO},           // 0x17
	{"JR 0x%02X", 1, TODO},      // 0x18
	{"ADD HL, DE", 0, TODO},     // 0x19
	{"LD A, (DE)", 0, [&](){ ld_A_atrw(&REG.DE); }},     // 0x1A
	{"DEC DE", 0, TODO},         // 0x1B
	{"INC E", 0, [&](){ inc_rb(&REG.E); }},          // 0x1C
	{"DEC E", 0, TODO},          // 0x1D
	{"LD E, 0x%02X", 1, [&](){ ld_rb_n(&REG.E); }},   // 0x1E
	{"RR A", 0, TODO},           // 0x1F

	{"JR NZ, 0x%02X", 1, TODO},       // 0x20
	{"LD HL, 0x%04X", 2, [&](){ ld_rw_nn(&REG.HL); }},  // 0x21
	{"LD (DE), A", 0, [&](){ ld_atrw_A(&REG.DE); }},     // 0x22
	{"INC HL", 0, [&](){ inc_rw(&REG.HL); }},         // 0x23
	{"INC H", 0, [&](){ inc_rb(&REG.H); }},          // 0x24
	{"DEC H", 0, TODO},          // 0x25
	{"LD H, 0x%02X", 1, [&](){ ld_rb_n(&REG.H); }},   // 0x26
	{"DAA", 0, TODO},            // 0x27
	{"JR Z, 0x%02X", 1, TODO},   // 0x28
	{"ADD HL, HL", 0, TODO},     // 0x29
	{"LDI A, (HL)", 0, [&](){ ldi_A_atHL(); }},    // 0x2A
	{"DEC HL", 0, TODO},         // 0x2B
	{"INC L", 0, [&](){ inc_rb(&REG.L); }},          // 0x2C
	{"DEC L", 0, TODO},          // 0x2D
	{"LD L, 0x%02X", 1, [&](){ ld_rb_n(&REG.L); }},   // 0x2E
	{"CPL", 0, TODO},            // 0x2F
 
	{"JR NC, 0x%02X", 1, TODO},  // 0x30
	{"LD SP, 0x%04X", 2, [&](){ ld_rw_nn(&REG.SP); }},  // 0x31
	{"LDD (HL), A", 0, [&](){ ldd_atHL_A(); }},    // 0x32
	{"INC SP", 0, [&](){ inc_rw(&REG.SP); }},         // 0x33
	{"INC (HL)", 0, [&](){ inc_atHL(); }},       // 0x34
	{"DEC (HL)", 0, TODO},       // 0x35
	{"LD (HL), 0x%02X", 1, [&](){ ld_atHL_n(); }},// 0x36
	{"SCF", 0, TODO},            // 0x37
	{"JR C, 0x%02X", 1, TODO},   // 0x38
	{"ADD HL, SP", 0, TODO},     // 0x39
	{"LDD A, (HL)", 0, [&](){ ldd_A_atHL(); }},    // 0x3A
	{"DEC SP", 0, TODO},         // 0x3B
	{"INC A", 0, TODO},          // 0x3C
	{"DEC A", 0, TODO},          // 0x3D
	{"LD A, 0x%02X", 1, [&](){ ld_rb_n(&REG.A); }},   // 0x3E
	{"CCF", 0, TODO},            // 0x3F

	{"LD B, B", 0, [&](){ ld_rb_rb(&REG.B, &REG.B); }},        // 0x40
	{"LD B, C", 0, [&](){ ld_rb_rb(&REG.B, &REG.C); }},        // 0x41
	{"LD B, D", 0, [&](){ ld_rb_rb(&REG.B, &REG.D); }},        // 0x42
	{"LD B, E", 0, [&](){ ld_rb_rb(&REG.B, &REG.E); }},        // 0x43
	{"LD B, H", 0, [&](){ ld_rb_rb(&REG.B, &REG.H); }},        // 0x44
	{"LD B, L", 0, [&](){ ld_rb_rb(&REG.B, &REG.L); }},        // 0x45
	{"LD B, (HL)", 0, [&](){ ld_rb_atHL(&REG.B); }},     // 0x46
	{"LD B, A", 0, [&](){ ld_rb_rb(&REG.B, &REG.A); }},        // 0x47
	{"LD C, B", 0, [&](){ ld_rb_rb(&REG.C, &REG.B); }},        // 0x48
	{"LD C, C", 0, [&](){ ld_rb_rb(&REG.C, &REG.C);}},        // 0x49
	{"LD C, D", 0, [&](){ ld_rb_rb(&REG.C, &REG.D);}},        // 0x4A
	{"LD C, E", 0, [&](){ ld_rb_rb(&REG.C, &REG.E);}},        // 0x4B
	{"LD C, H", 0, [&](){ ld_rb_rb(&REG.C, &REG.H);}},        // 0x4C
	{"LD C, L", 0, [&](){ ld_rb_rb(&REG.C, &REG.L);}},        // 0x4D
	{"LD C, (HL)", 0, [&](){ ld_rb_atHL(&REG.C); }},     // 0x4E
	{"LD C, A", 0, [&](){ ld_rb_rb(&REG.C, &REG.A);}},        // 0x4F

	{"LD D, B", 0, [&](){ ld_rb_rb(&REG.D, &REG.B);}},        // 0x50
	{"LD D, C", 0, [&](){ ld_rb_rb(&REG.D, &REG.C);}},        // 0x51
	{"LD D, D", 0, [&](){ ld_rb_rb(&REG.D, &REG.D);}},        // 0x52
	{"LD D, E", 0, [&](){ ld_rb_rb(&REG.D, &REG.E);}},        // 0x53
	{"LD D, H", 0, [&](){ ld_rb_rb(&REG.D, &REG.H);}},        // 0x54
	{"LD D, L", 0, [&](){ ld_rb_rb(&REG.D, &REG.L);}},        // 0x55
	{"LD D, (HL)", 0, [&](){ ld_rb_atHL(&REG.D); }},     // 0x56
	{"LD D, A", 0, [&](){ ld_rb_rb(&REG.D, &REG.A);}},        // 0x57
	{"LD E, B", 0, [&](){ ld_rb_rb(&REG.E, &REG.B);}},        // 0x58
	{"LD E, C", 0, [&](){ ld_rb_rb(&REG.E, &REG.C);}},        // 0x59
	{"LD E, D", 0, [&](){ ld_rb_rb(&REG.E, &REG.D);}},        // 0x5A
	{"LD E, E", 0, [&](){ ld_rb_rb(&REG.E, &REG.E);}},        // 0x5B
	{"LD E, H", 0, [&](){ ld_rb_rb(&REG.E, &REG.H);}},        // 0x5C
	{"LD E, L", 0, [&](){ ld_rb_rb(&REG.E, &REG.L);}},        // 0x5D
	{"LD E, (HL)", 0, [&](){ ld_rb_atHL(&REG.E); }},     // 0x5E
	{"LD E, A", 0, [&](){ ld_rb_rb(&REG.E, &REG.D);}},        // 0x5F

	{"LD H, B", 0, [&](){ ld_rb_rb(&REG.H, &REG.B);}},        // 0x60
	{"LD H, C", 0, [&](){ ld_rb_rb(&REG.H, &REG.C);}},        // 0x61
	{"LD H, D", 0, [&](){ ld_rb_rb(&REG.H, &REG.D);}},        // 0x62
	{"LD H, E", 0, [&](){ ld_rb_rb(&REG.H, &REG.E);}},        // 0x63
	{"LD H, H", 0, [&](){ ld_rb_rb(&REG.H, &REG.H);}},        // 0x64
	{"LD H, L", 0, [&](){ ld_rb_rb(&REG.H, &REG.L);}},        // 0x65
	{"LD H, (HL)", 0, [&](){ ld_rb_atHL(&REG.H); }},     // 0x66
	{"LD H, A", 0, [&](){ ld_rb_rb(&REG.H, &REG.A);}},        // 0x67
	{"LD L, B", 0, [&](){ ld_rb_rb(&REG.L, &REG.B);}},        // 0x68
	{"LD L, C", 0, [&](){ ld_rb_rb(&REG.L, &REG.C);}},        // 0x69
	{"LD L, D", 0, [&](){ ld_rb_rb(&REG.L, &REG.D);}},        // 0x6A
	{"LD L, E", 0, [&](){ ld_rb_rb(&REG.L, &REG.E);}},        // 0x6B
	{"LD L, H", 0, [&](){ ld_rb_rb(&REG.L, &REG.H);}},        // 0x6C
	{"LD L, L", 0, [&](){ ld_rb_rb(&REG.L, &REG.L);}},        // 0x6D
	{"LD L, (HL)", 0, [&](){ ld_rb_atHL(&REG.L); }},     // 0x6E
	{"LD L, A", 0, [&](){ ld_rb_rb(&REG.L, &REG.A);}},        // 0x6F

	{"LD (HL), B", 0, [&](){ ld_atHL_rb(&REG.B); }},     // 0x70
	{"LD (HL), C", 0, [&](){ ld_atHL_rb(&REG.C); }},     // 0x71
	{"LD (HL), D", 0, [&](){ ld_atHL_rb(&REG.D); }},     // 0x72
	{"LD (HL), E", 0, [&](){ ld_atHL_rb(&REG.E); }},     // 0x73
	{"LD (HL), H", 0, [&](){ ld_atHL_rb(&REG.H); }},     // 0x74
	{"LD (HL), L", 0, [&](){ ld_atHL_rb(&REG.L); }},     // 0x75
	{"HALT", 0, TODO},           // 0x76
	{"LD (HL), A", 0, [&](){ ld_atHL_rb(&REG.A); }},     // 0x77
	{"LD A, B", 0, [&](){ ld_rb_rb(&REG.A, &REG.B);}},        // 0x78
	{"LD A, C", 0, [&](){ ld_rb_rb(&REG.A, &REG.C);}},        // 0x79
	{"LD A, D", 0, [&](){ ld_rb_rb(&REG.A, &REG.D);}},        // 0x7A
	{"LD A, E", 0, [&](){ ld_rb_rb(&REG.A, &REG.E);}},        // 0x7B
	{"LD A, H", 0, [&](){ ld_rb_rb(&REG.A, &REG.H);}},        // 0x7C
	{"LD A, L", 0, [&](){ ld_rb_rb(&REG.A, &REG.L);}},        // 0x7D
	{"LD A, (HL)", 0, [&](){ ld_rb_atHL(&REG.L); }},     // 0x7E
	{"LD A, A", 0, [&](){ ld_rb_rb(&REG.A, &REG.A);}},        // 0x7F

	{"ADD A, B", 0, TODO},       // 0x80
	{"ADD A, C", 0, TODO},       // 0x81
	{"ADD A, D", 0, TODO},       // 0x82
	{"ADD A, E", 0, TODO},       // 0x83
	{"ADD A, H", 0, TODO},       // 0x84
	{"ADD A, L", 0, TODO},       // 0x85
	{"ADD A, (HL)", 0, TODO},    // 0x86
	{"ADD A, A", 0, TODO},       // 0x87
	{"ADC A, B", 0, TODO},       // 0x88
	{"ADC A, C", 0, TODO},       // 0x89
	{"ADC A, D", 0, TODO},       // 0x8A
	{"ADC A, E", 0, TODO},       // 0x8B
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
	{"SBC A, B", 0, TODO},       // 0x98
	{"SBC A, C", 0, TODO},       // 0x99
	{"SBC A, D", 0, TODO},       // 0x9A
	{"SBC A, E", 0, TODO},       // 0x9B
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
	{"XOR B", 0, [&](){ xor_rb(&REG.B); }},          // 0xA8
	{"XOR C", 0, [&](){ xor_rb(&REG.C);}},          // 0xA9
	{"XOR D", 0, [&](){ xor_rb(&REG.D);}},          // 0xAA
	{"XOR E", 0, [&](){ xor_rb(&REG.E);}},          // 0xAB
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
	{"CP B", 0, [&](){ cp_rb(&REG.B); }},           // 0xB8
	{"CP C", 0, [&](){ cp_rb(&REG.C); }},           // 0xB9
	{"CP D", 0, [&](){ cp_rb(&REG.D); }},           // 0xBA
	{"CP E", 0, [&](){ cp_rb(&REG.E); }},           // 0xBB
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
	{"RET Z", 0, TODO},          // 0xC8
	{"RET", 0, TODO},            // 0xC9
	{"JP Z, 0x%04X", 2, TODO},   // 0xCA
	{"Ext Op", 1, [&](){ ext(); }},         // 0xCB
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
	{"RET C", 0, TODO},          // 0xD8
	{"RETI", 0, TODO},            // 0xD9
	{"JP C, 0x%04X", 2, TODO},   // 0xDA
	{"XX", 0, TODO},         // 0xDB
	{"CALL C, 0x%04X", 2, TODO}, // 0xDC
	{"XX", 0, TODO},    // 0xDD
	{"SBC A, 0x%02X", 1, TODO},  // 0xDE
	{"RST 18", 0, TODO},          // 0xDF

	{"LDH (0x%02X), A", 1, [&](){ ldh_atn_A(); }},         // 0xE0
	{"POP HL", 0, TODO},         // 0xE1
	{"LDH (C), A", 0, [&](){ ldh_atC_A(); } },  // 0xE2
	{"XX", 0, TODO},      // 0xE3
	{"XX", 0, TODO},// 0xE4
	{"PUSH HL", 0, TODO},        // 0xE5
	{"AND n", 1, [&](){ and_n(); }},  // 0xE6
	{"RST 20", 0, TODO},          // 0xE7
	{"ADD SP, d", 0, TODO},          // 0xE8
	{"JP (HL)", 0, TODO},            // 0xE9
	{"LD (0x%04X), A", 2, TODO},   // 0xEA
	{"XX", 0, TODO},         // 0xEB
	{"XX", 0, TODO}, // 0xEC
	{"XX", 0, TODO},    // 0xED
	{"XOR n", 1, [&](){ xor_n(); }},  // 0xEE
	{"RST 28", 0, TODO},          // 0xEF

	{"LDH A, (0x%02X)", 1, [&](){ ldh_A_atn(); }},         // 0xF0
	{"POP AF", 0, TODO},         // 0xF1
	{"XX", 0, TODO},  // 0xF2
	{"DI", 0, TODO},      // 0xF3
	{"XX", 0, TODO},// 0xF4
	{"PUSH AF", 0, TODO},        // 0xF5
	{"OR n", 1, [&](){ or_n(); }},  // 0xF6
	{"RST 30", 0, TODO},          // 0xF7
	{"LDHL SP, d", 0, TODO},          // 0xF8
	{"LD SP, HL", 0, [&](){ ld_SP_HL(); }},            // 0xF9
	{"LD A, (0x%04X)", 2, TODO},   // 0xFA
	{"EI", 0, TODO},         // 0xFB
	{"XX", 0, TODO}, // 0xFC
	{"XX", 0, TODO},    // 0xFD
	{"CP n", 1, [&](){ cp_n(); }},  // 0xFE
	{"RST 38", 0, TODO}          // 0xFF
};