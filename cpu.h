#pragma once
#include <inttypes.h>
#include <functional>
#include <algorithm>

#include "reg.h"

#define ISR_VBLANK 0x0040
#define ISR_LCD    0x0048
#define ISR_TIMER  0x0050
#define ISR_SERIAL 0x0058
#define ISR_JOYPAD 0x0060

#define FLAG_IF_VBLANK 0x01
#define FLAG_IF_LCD    0x02
#define FLAG_IF_TIMER  0x04
#define FLAG_IF_SERIAL 0x08
#define FLAG_IF_JOYPAD 0x10

class Memory;

class Cpu {
public:
	typedef struct {
		char name[16];
		uint8_t argw;
		void (*fn)(Cpu&);
	} Instruction;

	Cpu(Memory &MemRef, Registers &RegRef) : MEM(MemRef), REG(RegRef) {}

	Memory &MEM;
	Registers &REG;

	uint8_t readByte(uint16_t addr);
	uint16_t readWord(uint16_t addr);

	void writeByte(uint16_t addr, uint8_t val);
	void writeWord(uint16_t addr, uint16_t val);

	void handle_interrupts();

	uint8_t argbyte() {
		REG.PC += 1;
		return readByte(REG.PC - 1);
	}

	uint16_t argword() {
		REG.PC += 2;
		return readWord(REG.PC - 2);
	}

	void nop() {
		REG.TCLK = 4;
	}

	// 8-bit arithmetic

	void inc_rb(uint8_t * ptr) {
		(*ptr)++;

		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_H, (*ptr & 0x0F) == 0);
		REG.set_flag_cond(FLAG_Z, *ptr == 0);
		REG.TCLK = 4;
	}

	void inc_atHL() {
		uint8_t val = readByte(REG.HL);
		writeByte(REG.HL, val + 1);

		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_H, (val & 0x0F) == 0x0F);
		REG.set_flag_cond(FLAG_Z, (val == 0xFF));

		REG.TCLK = 12;
	}

	void dec_rb(uint8_t * ptr) {
		(*ptr)--;

		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_H, (*ptr & 0x0F) == 0x0F);
		REG.set_flag_cond(FLAG_Z, *ptr == 0);
		REG.TCLK = 4;
	}

	void dec_atHL() {
		uint8_t val = readByte(REG.HL);
		writeByte(REG.HL, val - 1);

		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_H, (val & 0x0F) == 0x00);
		REG.set_flag_cond(FLAG_Z, (val - 1) == 0);

		REG.TCLK = 12;
	}

	void xor_rb(uint8_t * from) {
		REG.A ^= *from;
		
		REG.unset_flag(FLAG_H | FLAG_N | FLAG_C);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 4;
	} 

	void xor_n() {
		REG.A ^= argbyte();
		
		REG.unset_flag(FLAG_H | FLAG_N | FLAG_C);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 8;
	}

	void xor_atHL() {
		REG.A ^= readByte(REG.HL);
		
		REG.unset_flag(FLAG_H | FLAG_N | FLAG_C);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 8;
	}

	void or_rb(uint8_t * from) {
		REG.A |= *from;
		
		REG.unset_flag(FLAG_H | FLAG_N | FLAG_C);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 4;
	} 

	void or_n() {
		REG.A |= argbyte();
		
		REG.unset_flag(FLAG_H | FLAG_N | FLAG_C);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 8;
	}

	void or_atHL() {
		REG.A |= readByte(REG.HL);
		
		REG.unset_flag(FLAG_H | FLAG_N | FLAG_C);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 8;
	}

	void and_rb(uint8_t * from) {
		REG.A &= *from;
		
		REG.unset_flag(FLAG_N | FLAG_C);
		REG.set_flag(FLAG_H);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 4;
	} 

	void and_n() {
		REG.A &= argbyte();
		
		REG.unset_flag(FLAG_N | FLAG_C);
		REG.set_flag(FLAG_H);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 8;
	}

	void and_atHL() {
		REG.A &= readByte(REG.HL);
		
		REG.unset_flag(FLAG_N | FLAG_C);
		REG.set_flag(FLAG_H);
		if (REG.A == 0) REG.set_flag(FLAG_Z);
		else REG.unset_flag(FLAG_Z);

		REG.TCLK = 8;
	}

	void cp_rb(uint8_t * from) {

		uint8_t e = *from;

		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, e == REG.A);
		REG.set_flag_cond(FLAG_C, e > REG.A);
		REG.set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

		REG.TCLK = 4;
	} 

	void cp_n() {

		uint8_t e = argbyte();

		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, e == REG.A);
		REG.set_flag_cond(FLAG_C, e > REG.A);
		REG.set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

		REG.TCLK = 8;
	}

	void cp_atHL() {

		uint8_t e = readByte(REG.HL);

		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, e == REG.A);
		REG.set_flag_cond(FLAG_C, e > REG.A);
		REG.set_flag_cond(FLAG_H, (e & 0x0F) > (REG.A & 0x0F));

		REG.TCLK = 8;
	}

	void add_A_rb(uint8_t * ptr) {
		REG.set_flag_cond(FLAG_C, 0xFF - REG.A < *ptr);
		REG.set_flag_cond(FLAG_H, ((REG.A & 0x0F) + (*ptr & 0x0F)) & 0xF0);
		REG.A += *ptr;
		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 4;
	}

	void add_A_n() {
		uint8_t val = argbyte();
		REG.set_flag_cond(FLAG_C, 0xFF - REG.A < val);
		REG.set_flag_cond(FLAG_H, ((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
		REG.A += val;
		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void add_A_atHL() {
		uint8_t val = readByte(REG.HL);

		REG.set_flag_cond(FLAG_C, 0xFF - REG.A < val);
		REG.set_flag_cond(FLAG_H, ((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
		REG.A += val;
		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void add_SP_e() {
		uint8_t n = argbyte();
		int8_t e = *reinterpret_cast<uint8_t*>(&n);

		uint8_t lb = REG.SP & 0x00FF;

		REG.set_flag_cond(FLAG_C, (0xFF - lb) < n);
		REG.set_flag_cond(FLAG_H, ((lb & 0x0F) + (n & 0x0F)) & 0x10);
		REG.SP += e;

		REG.unset_flag(FLAG_Z | FLAG_N);

		REG.TCLK = 16;
	}

	void adc_A_rb(uint8_t * ptr) {
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, 0xFF - REG.A < *ptr);
		REG.set_flag_cond(FLAG_H, ((REG.A & 0x0F) + (*ptr & 0x0F)) & 0xF0);
		REG.A += *ptr;
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (0xFF - REG.A < carry));
		REG.set_flag_cond(FLAG_H, REG.get_flag(FLAG_H) || (((REG.A & 0x0F) + carry) & 0xF0));
		REG.A += carry;
		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 4;
	}

	void adc_A_n() {
		uint8_t val = argbyte();
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, 0xFF - REG.A < val);
		REG.set_flag_cond(FLAG_H, ((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
		REG.A += val;
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (0xFF - REG.A < carry));
		REG.set_flag_cond(FLAG_H, REG.get_flag(FLAG_H) || (((REG.A & 0x0F) + carry) & 0xF0));
		REG.A += carry;
		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void adc_A_atHL() {
		uint8_t val = readByte(REG.HL);
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, 0xFF - REG.A < val);
		REG.set_flag_cond(FLAG_H, ((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
		REG.A += val;
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (0xFF - REG.A < carry));
		REG.set_flag_cond(FLAG_H, REG.get_flag(FLAG_H) || (((REG.A & 0x0F) + carry) & 0xF0));
		REG.A += carry;
		REG.unset_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void sub_A_rb(uint8_t * ptr) {
		REG.set_flag_cond(FLAG_C, REG.A < *ptr);
		REG.set_flag_cond(FLAG_H, (REG.A & 0x0F) < (*ptr & 0x0F));
		REG.A -= *ptr;
		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 4;
	}

	void sub_A_n() {
		uint8_t val = argbyte();
		REG.set_flag_cond(FLAG_C, REG.A < val);
		REG.set_flag_cond(FLAG_H, (REG.A & 0x0F) < (val & 0x0F));
		REG.A -= val;
		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void sub_A_atHL() {
		uint8_t val = readByte(REG.HL);
		REG.set_flag_cond(FLAG_C, REG.A < val);
		REG.set_flag_cond(FLAG_H, (REG.A & 0x0F) < (val & 0x0F));
		REG.A -= val;
		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void sbc_A_rb(uint8_t * ptr) {
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, REG.A < *ptr);
		REG.set_flag_cond(FLAG_H, (REG.A & 0x0F) < (*ptr & 0x0F));
		REG.A -= *ptr;
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (REG.A < carry));
		REG.set_flag_cond(FLAG_H, REG.get_flag(FLAG_H) || (((REG.A & 0x0F) < carry)));
		REG.A -= carry;
		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 4;
	}

	void sbc_A_n() {
		bool carry = REG.get_flag(FLAG_C);
		uint8_t val = argbyte();
		REG.set_flag_cond(FLAG_C, REG.A < val);
		REG.set_flag_cond(FLAG_H, (REG.A & 0x0F) < (val & 0x0F));
		REG.A -= val;
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (REG.A < carry));
		REG.set_flag_cond(FLAG_H, REG.get_flag(FLAG_H) || (((REG.A & 0x0F) < carry)));
		REG.A -= carry;
		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}

	void sbc_A_atHL() {
		bool carry = REG.get_flag(FLAG_C);
		uint8_t val = readByte(REG.HL);
		REG.set_flag_cond(FLAG_C, REG.A < val);
		REG.set_flag_cond(FLAG_H, (REG.A & 0x0F) < (val & 0x0F));
		REG.A -= val; 
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (REG.A < carry));
		REG.set_flag_cond(FLAG_H, REG.get_flag(FLAG_H) || (((REG.A & 0x0F) < carry)));
		REG.A -= carry;
		REG.set_flag(FLAG_N);
		REG.set_flag_cond(FLAG_Z, REG.A == 0);

		REG.TCLK = 8;
	}


	// 16-bit arithmetic

	void inc_rw(uint16_t * ptr) {
		(*ptr)++;
		REG.TCLK = 8;	
	}

	void dec_rw(uint16_t * ptr) {
		(*ptr)--;
		REG.TCLK = 8;	
	}

	void add_hl_rw(uint16_t * ptr) {
		REG.set_flag_cond(FLAG_C, 0xFFFF - REG.HL < *ptr);
		REG.set_flag_cond(FLAG_H, ((REG.HL & 0x0FFF) + (*ptr & 0x0FFF)) & 0x1000);
		REG.HL += *ptr;
		REG.unset_flag(FLAG_N);

		REG.TCLK = 8;
	}

	// 8-bit loads

	void ld_rb_rb(uint8_t *to, uint8_t *from) {
		(*to) = (*from);
		REG.TCLK = 4;	
	}

	void ld_rb_n(uint8_t *to) {
		(*to) = argbyte();
		REG.TCLK = 8;	
	}

	void ld_rb_atHL(uint8_t *to) {
		(*to) = readByte(REG.HL);
		REG.TCLK = 12;	
	}

	void ld_atHL_rb(uint8_t *from) {
		writeByte(REG.HL, *from);
		REG.TCLK = 8;
	}

	void ld_atHL_n() {
		writeByte(REG.HL, argbyte());
		REG.TCLK = 12;
	}

	void ld_A_atrw(uint16_t *addr) {
		REG.A = readWord(*addr);
		REG.TCLK = 8;
	}

	void ld_A_atnn() {
		REG.A = readByte(argword());
		REG.TCLK = 16;
	}

	void ld_atrw_A(uint16_t *addr) {
		writeByte(*addr, REG.A);
		REG.TCLK = 8;
	}

	void ld_atnn_A() {
		writeByte(argword(), REG.A);
		REG.TCLK = 16;
	}

	void ldh_A_atC() {
		REG.A = readByte(0xFF00 | REG.C);
		REG.TCLK = 8;
	}

	void ldh_atC_A() {
		writeByte(0xFF00| REG.C, REG.A);
		REG.TCLK = 8;
	}

	void ldd_A_atHL() {
		REG.A = readByte(REG.HL--);
		REG.TCLK = 8;
	}

	void ldd_atHL_A() {
		writeByte(REG.HL--, REG.A);
		REG.TCLK = 8;
	}

	void ldi_A_atHL() {
		REG.A = readByte(REG.HL++);
		REG.TCLK = 8;
	}

	void ldi_atHL_A() {
		writeByte(REG.HL++, REG.A);
		REG.TCLK = 8;
	}

	void ldh_atn_A() {
		writeByte(0xFF00 | argbyte(), REG.A);
		REG.TCLK = 12;
	}

	void ldh_A_atn() {
		REG.A = readByte(0xFF00 | argbyte());
		REG.TCLK = 12;
	}

	// 16-bit loads

	void ld_rw_nn(uint16_t* to) {
		*to = argword();
		REG.TCLK = 12;
	}

	void ld_atnn_SP() {
		uint16_t addr = argword();
		writeWord(addr, REG.SP);
		REG.TCLK = 20;
	}

	void ld_SP_HL() {
		REG.SP = REG.HL;
		REG.TCLK = 8;
	}

	void ld_HL_SP_e() {

		uint8_t n = argbyte();
		int8_t e = *reinterpret_cast<uint8_t*>(&n);

		REG.HL = REG.SP + e;

		// TODO fix H, C flags
		REG.set_flag_cond(FLAG_H, 0x000F - (REG.SP & 0x000F) < (e & 0x000F) );
		REG.set_flag_cond(FLAG_C, 0x00FF - (REG.SP & 0x00FF) < (e & 0x00FF) );
		REG.unset_flag(FLAG_Z | FLAG_N);
	}

	void push_rw(uint16_t *at) {
		REG.SP -= 2;
		writeWord(REG.SP, *at);
		REG.TCLK = 16;
	}

	void pop_rw(uint16_t *at) {
		*at = readWord(REG.SP);
		REG.SP += 2;
		REG.TCLK = 12;
	}

	// only top 4 bits of F are writable
	void pop_AF() {
		REG.AF &= 0x000F;
		REG.AF |= (0xFFF0 & readWord(REG.SP));
		REG.SP += 2;
		REG.TCLK = 12;
	}

	// ext block

	// rotate right
	void rrca() {
		bool carry = REG.A & BIT_0;
		REG.set_flag_cond(FLAG_C, carry);
		REG.A >>= 1;
		if (carry) REG.A |= BIT_7;

		REG.unset_flag(FLAG_N | FLAG_H | FLAG_Z);

		REG.TCLK = 4;
	}

	void rrc_rb(uint8_t * at) {
		bool carry = *at & BIT_0;
		REG.set_flag_cond(FLAG_C, carry);
		*at >>= 1;
		if (carry) *at |= BIT_7;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void rrc_atHL() {
		uint8_t val = readByte(REG.HL);
		bool carry = val & BIT_0;
		REG.set_flag_cond(FLAG_C, carry);
		val >>= 1;
		if (carry) val |= BIT_7;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	// rotate left
	void rlca() {
		bool carry = REG.A & BIT_7;
		REG.set_flag_cond(FLAG_C, carry);
		REG.A <<= 1;
		if (carry) REG.A |= BIT_0;

		REG.unset_flag(FLAG_N | FLAG_H | FLAG_Z);

		REG.TCLK = 4;
	}

	void rlc_rb(uint8_t * at) {
		bool carry = *at & BIT_7;
		REG.set_flag_cond(FLAG_C, carry);
		*at <<= 1;
		if (carry) *at |= BIT_0;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void rlc_atHL() {
		uint8_t val = readByte(REG.HL);
		bool carry = val & BIT_7;
		REG.set_flag_cond(FLAG_C, carry);
		val <<= 1;
		if (carry) val |= BIT_0;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	// rotate right (through carry flag)
	void rra() {
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, REG.A & BIT_0);
		REG.A >>= 1;
		if (carry) REG.A |= BIT_7;

		REG.unset_flag(FLAG_N | FLAG_H | FLAG_Z);

		REG.TCLK = 4;
	}

	void rr_rb(uint8_t * at) {
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, *at & BIT_0);
		*at >>= 1;
		if (carry) *at |= BIT_7;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void rr_atHL() {
		uint8_t val = readByte(REG.HL);
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, val & BIT_0);
		val >>= 1;
		if (carry) val |= BIT_7;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	// rotate left (through carry flag)
	void rla() {
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, REG.A & BIT_7);
		REG.A <<= 1;
		if (carry) REG.A |= BIT_0;

		REG.unset_flag(FLAG_N | FLAG_H | FLAG_Z);

		REG.TCLK = 4;
	}

	void rl_rb(uint8_t * at) {
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, *at & BIT_7);
		*at <<= 1;
		if (carry) *at |= BIT_0;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void rl_atHL() {
		uint8_t val = readByte(REG.HL);
		bool carry = REG.get_flag(FLAG_C);
		REG.set_flag_cond(FLAG_C, val & BIT_7);
		val <<= 1;
		if (carry) val |= BIT_0;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	// shift left
	void sla_rb(uint8_t * at) {
		REG.set_flag_cond(FLAG_C, *at & BIT_7);
		*at <<= 1;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void sla_atHL() {
		uint8_t val = readByte(REG.HL);
		REG.set_flag_cond(FLAG_C, val & BIT_7);
		val <<= 1;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	// shift right (keep bit 7)
	void sra_rb(uint8_t * at) {
		REG.set_flag_cond(FLAG_C, *at & BIT_0);
		*at >>= 1;
		if (*at & BIT_6) *at |= BIT_7;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void sra_atHL() {
		uint8_t val = readByte(REG.HL);
		REG.set_flag_cond(FLAG_C, val & BIT_0);
		val >>= 1;
		if (val & BIT_6) val |= BIT_7;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	// shift right
	void srl_rb(uint8_t * at) {
		REG.set_flag_cond(FLAG_C, *at & BIT_0);
		*at >>= 1;

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void srl_atHL() {
		uint8_t val = readByte(REG.HL);
		REG.set_flag_cond(FLAG_C, val & BIT_0);
		val >>= 1;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 8;
	}

	void swap_rb(uint8_t * at) {
		uint8_t tmp = *at << 4;
		*at >>= 4;
		*at |= tmp;

		REG.unset_flag(FLAG_N | FLAG_H | FLAG_C);
		REG.set_flag_cond(FLAG_Z, *at == 0);

		REG.TCLK = 8;
	}

	void swap_atHL() {
		uint8_t val = readByte(REG.HL);
		uint8_t tmp = val << 4;
		val >>= 4;
		val |= tmp;
		writeByte(REG.HL, val);

		REG.unset_flag(FLAG_N | FLAG_H | FLAG_C);
		REG.set_flag_cond(FLAG_Z, val == 0);

		REG.TCLK = 16;
	}

	void cpl() {
		REG.A = ~REG.A;
		REG.set_flag(FLAG_N | FLAG_H);

		REG.TCLK = 4;
	}

	void bit_i_rb(const uint8_t mask, uint8_t *from) {
		REG.unset_flag(FLAG_N);
		REG.set_flag(FLAG_H);
		REG.set_flag_cond(FLAG_Z, (*from & mask) == 0);

		REG.TCLK = 8;
	}

	void bit_i_atHL(const uint8_t mask) {
		REG.unset_flag(FLAG_N);
		REG.set_flag(FLAG_H);
		REG.set_flag_cond(FLAG_Z, (readByte(REG.HL) & mask) == 0);

		REG.TCLK = 12;
	}

	void res_i_rb(const uint8_t mask, uint8_t * to) {
		*to &= ~mask;
		REG.TCLK = 8;
	}

	void res_i_atHL(const uint8_t mask) {
		uint8_t val = readByte(REG.HL);
		val &= ~mask;
		writeByte(REG.HL, val);

		REG.TCLK = 16;
	}

	void set_i_rb(const uint8_t mask, uint8_t * to) {
		*to |= mask;
		REG.TCLK = 8;
	}

	void set_i_atHL(const uint8_t mask) {
		uint8_t val = readByte(REG.HL);
		val |= mask;
		writeByte(REG.HL, val);

		REG.TCLK = 16;
	}

	// jumps

	void jp_nn() {
		REG.TCLK = 16;
		REG.PC = argword();
	}

	void jp_f_nn(uint8_t mask) {
		bool cond = REG.get_flag(mask);
		uint16_t addr = argword();
		if (cond) {
			REG.TCLK = 16;
			REG.PC = addr;
		} else {
			REG.TCLK = 12;
		}
	}

	void jp_nf_nn(uint8_t mask) {
		bool cond = !REG.get_flag(mask);
		uint16_t addr = argword();
		if (cond) {
			REG.TCLK = 16;
			REG.PC = addr;
		} else {
			REG.TCLK = 12;
		}
	}

	void jp_atHL() {
		REG.TCLK = 4;
		REG.PC = REG.HL;
	}

	void jr_e() {
		uint8_t n = argbyte();
		int8_t e = *reinterpret_cast<int8_t*>(&n);
		REG.PC += e;

		REG.TCLK = 12;
	}

	void jr_f_e(uint8_t mask) {
		bool cond = REG.get_flag(mask);
		uint8_t n = argbyte();
		if (cond) {
			int8_t e = *reinterpret_cast<int8_t*>(&n);
			REG.TCLK = 12;
			#ifndef NDEBUG
			if (e == -2) {
				printf("DEBUG: exiting on JRO 0\n");
				exit(1);
			}
			#endif
			REG.PC += e;
		} else {
			REG.TCLK = 8;
		}
	}

	void jr_nf_e(uint8_t mask) {
		bool cond = !REG.get_flag(mask);
		uint8_t n = argbyte();
		if (cond) {
			int8_t e = *reinterpret_cast<int8_t*>(&n);
			REG.TCLK = 12;
			#ifndef NDEBUG
			if (e == -2) {
				printf("DEBUG: exiting on JRO 0\n");
				exit(1);
			}
			#endif
			REG.PC += e;
		} else {
			REG.TCLK = 8;
		}
	}

	// interrupts

	void ei() {
		REG.IME = 1;
		REG.TCLK = 4;
	}

	void di() {
		REG.IME = 0;
		REG.TCLK = 4;
	}

	// calls

	void call_nn() {
		uint16_t nn = argword();
		writeWord(REG.SP - 2, REG.PC);
		REG.PC = nn;
		REG.SP -= 2;
		REG.TCLK = 24;
	}

	void call_f_nn(uint8_t mask) {
		uint16_t nn = argword();
		bool cond = REG.get_flag(mask);
		if (cond) {
			writeWord(REG.SP - 2, REG.PC);
			REG.PC = nn;
			REG.SP -= 2;
			REG.TCLK = 24;
		} else {
			REG.TCLK = 12;
		}
	}

	void call_nf_nn(uint8_t mask) {
		uint16_t nn = argword();
		bool cond = !REG.get_flag(mask);
		if (cond) {
			writeWord(REG.SP - 2, REG.PC);
			REG.PC = nn;
			REG.SP -= 2;
			REG.TCLK = 24;
		} else {
			REG.TCLK = 12;
		}
	}

	// reset

	void rst(uint8_t addr) {
		writeWord(REG.SP - 2, REG.PC);
		REG.SP -= 2;
		REG.PC = addr;
		REG.TCLK = 12;
	}

	void rsti(uint8_t addr) {
		writeWord(REG.SP - 2, REG.PC);
		REG.SP -= 2;
		REG.PC = addr;
	}

	// ret

	void ret() {
		REG.PC = readWord(REG.SP);
		REG.SP += 2;
		REG.TCLK = 16;
	}

	void ret_f(uint8_t mask) {
		bool cond = REG.get_flag(mask);
		if (cond) {
			REG.PC = readWord(REG.SP);
			REG.SP += 2;
			REG.TCLK = 20;
		} else {
			REG.TCLK = 8;
		}
	}

	void ret_nf(uint8_t mask) {
		bool cond = !REG.get_flag(mask);
		if (cond) {
			REG.PC = readWord(REG.SP);
			REG.SP += 2;
			REG.TCLK = 20;
		} else {
			REG.TCLK = 8;
		}	
	}

	void reti() {
		REG.PC = readWord(REG.SP);
		REG.SP += 2;
		REG.TCLK = 16;
		REG.IME = 1;
	}

	void halt() {
		REG.HALT = 1;
		REG.TCLK = 16;
	}

	void daa() {

		int a = REG.A;

		if (!REG.get_flag(FLAG_N)) {
			if (REG.get_flag(FLAG_H) || (a & 0x0F) > 0x09)
				a += 0x06;			
			if (REG.get_flag(FLAG_C) || a > 0x9F)
				a += 0x60;
		} else {
			if (REG.get_flag(FLAG_H))
				a = (a - 6) & 0xFF;
			if (REG.get_flag(FLAG_C))
				a -= 0x60;
		}

		REG.unset_flag(FLAG_H);
		REG.set_flag_cond(FLAG_C, REG.get_flag(FLAG_C) || (a & 0x100) == 0x100);
		a &= 0xFF;
		REG.set_flag_cond(FLAG_Z, a == 0);

		REG.A = (uint8_t)a;
		REG.TCLK = 4;
	}

	void scf() {
		REG.set_flag(FLAG_C);
		REG.unset_flag(FLAG_H | FLAG_N);
		REG.TCLK = 4;
	}

	void ccf() {
		REG.set_flag_cond(FLAG_C, !REG.get_flag(FLAG_C));
		REG.unset_flag(FLAG_H | FLAG_N);
		REG.TCLK = 4;
	}

	void TODO() {
		printf("Operation not implemented.\nContext:\n");
		uint16_t start = (REG.PC >= 10) ? REG.PC - 10 : 0;
		for (uint16_t i = start; i < REG.PC; ++i)
			printf("%02X ", readByte(start + i));
		printf("| %02X | ", readByte(REG.PC));
		uint16_t end = (REG.PC < 0xFFFF - 10) ? REG.PC + 10 : 0xFFFF;
		for (uint16_t i = REG.PC + 1; i < end; ++i)
			printf("%02X ", readByte(i));
		printf("\n");

		exit(1);
	}

	Instruction ext_instructions[256] = {
		{"RLC B", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.B); }}, //0xCB00
		{"RLC C", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.C);}}, //0xCB01
		{"RLC D", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.D);}}, //0xCB02
		{"RLC E", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.E);}}, //0xCB03
		{"RLC H", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.H);}}, //0xCB04
		{"RLC L", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.L);}}, //0xCB05
		{"RLC (HL)", 0, [](Cpu &CPU){ CPU.rlc_atHL();}},  //0xCB06
		{"RLC A", 0, [](Cpu &CPU){ CPU.rlc_rb(&CPU.REG.A);}}, //0xCB07
		{"RRC B", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.B);}}, //0xCB08
		{"RRC C", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.C);}}, //0xCB09
		{"RRC D", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.D);}}, //0xCB0A
		{"RRC E", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.E);}}, //0xCB0B
		{"RRC H", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.H);}}, //0xCB0C
		{"RRC L", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.L);}}, //0xCB0D 
		{"RRC (HL)", 0, [](Cpu &CPU){ CPU.rrc_atHL();}}, //0xCB0E
		{"RRC A", 0, [](Cpu &CPU){ CPU.rrc_rb(&CPU.REG.A);}}, //0xCB0F

		{"RL B", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.B); }}, //0xCB10
		{"RL C", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.C);}}, //0xCB11
		{"RL D", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.D);}}, //0xCB12
		{"RL E", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.E);}}, //0xCB13
		{"RL H", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.H);}}, //0xCB14
		{"RL L", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.L);}}, //0xCB15
		{"RL (HL)", 0, [](Cpu &CPU){ CPU.rl_atHL();}},  //0xCB16
		{"RL A", 0, [](Cpu &CPU){ CPU.rl_rb(&CPU.REG.A);}}, //0xCB17
		{"RR B", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.B);}}, //0xCB18
		{"RR C", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.C);}}, //0xCB19
		{"RR D", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.D);}}, //0xCB1A
		{"RR E", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.E);}}, //0xCB1B
		{"RR H", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.H);}}, //0xCB1C
		{"RR L", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.L);}}, //0xCB1D 
		{"RR (HL)", 0, [](Cpu &CPU){ CPU.rr_atHL();}},  //0xCB1E
		{"RR A", 0, [](Cpu &CPU){ CPU.rr_rb(&CPU.REG.A);}}, //0xCB1F

		{"SLA B", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.B); }}, //0xCB20
		{"SLA C", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.C);}}, //0xCB21
		{"SLA D", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.D);}}, //0xCB22
		{"SLA E", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.E);}}, //0xCB23
		{"SLA H", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.H);}}, //0xCB24
		{"SLA L", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.L);}}, //0xCB25
		{"SLA (HL)", 0, [](Cpu &CPU){ CPU.sla_atHL();}},  //0xCB26
		{"SLA A", 0, [](Cpu &CPU){ CPU.sla_rb(&CPU.REG.A);}}, //0xCB27
		{"SRA B", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.B);}}, //0xCB28
		{"SRA C", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.C);}}, //0xCB29
		{"SRA D", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.D);}}, //0xCB2A
		{"SRA E", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.E);}}, //0xCB2B
		{"SRA H", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.H);}}, //0xCB2C
		{"SRA L", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.L);}}, //0xCB2D 
		{"SRA (HL)", 0, [](Cpu &CPU){ CPU.sra_atHL();}},  //0xCB2E
		{"SRA A", 0, [](Cpu &CPU){ CPU.sra_rb(&CPU.REG.A);}}, //0xCB3F

		{"SWAP B", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.B); }}, //0xCB30
		{"SWAP C", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.C);}}, //0xCB31
		{"SWAP D", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.D);}}, //0xCB32
		{"SWAP E", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.E);}}, //0xCB33
		{"SWAP H", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.H);}}, //0xCB34
		{"SWAP L", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.L);}}, //0xCB35
		{"SWAP (HL)", 0, [](Cpu &CPU){ CPU.swap_atHL();}},  //0xCB36
		{"SWAP A", 0, [](Cpu &CPU){ CPU.swap_rb(&CPU.REG.A);}}, //0xCB37
		{"SRL B", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.B);}}, //0xCB38
		{"SRL C", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.C);}}, //0xCB39
		{"SRL D", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.D);}}, //0xCB3A
		{"SRL E", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.E);}}, //0xCB3B
		{"SRL H", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.H);}}, //0xCB3C
		{"SRL L", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.L);}}, //0xCB3D
		{"SRL (HL)", 0, [](Cpu &CPU){ CPU.srl_atHL();}},  //0xCB3E
		{"SRL A", 0, [](Cpu &CPU){ CPU.srl_rb(&CPU.REG.A);}}, //0xCB3F

		{"BIT 0, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.B); }}, //0xCB40
		{"BIT 0, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.C);}}, //0xCB41
		{"BIT 0, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.D);}}, //0xCB42
		{"BIT 0, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.E);}}, //0xCB43
		{"BIT 0, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.H);}}, //0xCB44
		{"BIT 0, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.L);}}, //0xCB45
		{"BIT 0, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_0);}}, //0xCB46
		{"BIT 0, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_0, &CPU.REG.A);}}, //0xCB47
		{"BIT 1, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.B);}}, //0xCB48
		{"BIT 1, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.C);}}, //0xCB49
		{"BIT 1, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.D);}}, //0xCB4A
		{"BIT 1, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.E);}}, //0xCB4B
		{"BIT 1, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.H);}}, //0xCB4C
		{"BIT 1, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.L);}}, //0xCB4D
		{"BIT 1, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_1);}}, //0xCB4E
		{"BIT 1, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_1, &CPU.REG.A);}}, //0xCB4F

		{"BIT 2, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.B); }}, //0xCB50
		{"BIT 2, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.C);}}, //0xCB51
		{"BIT 2, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.D);}}, //0xCB52
		{"BIT 2, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.E);}}, //0xCB53
		{"BIT 2, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.H);}}, //0xCB54
		{"BIT 2, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.L);}}, //0xCB55
		{"BIT 2, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_2);}}, //0xCB56
		{"BIT 2, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_2, &CPU.REG.A);}}, //0xCB57
		{"BIT 3, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.B);}}, //0xCB58
		{"BIT 3, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.C);}}, //0xCB59
		{"BIT 3, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.D);}}, //0xCB5A
		{"BIT 3, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.E);}}, //0xCB5B
		{"BIT 3, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.H);}}, //0xCB5C
		{"BIT 3, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.L);}}, //0xCB5D 
		{"BIT 3, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_3);}}, //0xCB5E
		{"BIT 3, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_3, &CPU.REG.A);}}, //0xCB5F

		{"BIT 4, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.B); }}, //0xCB60
		{"BIT 4, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.C);}}, //0xCB61
		{"BIT 4, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.D);}}, //0xCB62
		{"BIT 4, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.E);}}, //0xCB63
		{"BIT 4, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.H);}}, //0xCB64
		{"BIT 4, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.L);}}, //0xCB65
		{"BIT 4, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_4);}}, //0xCB66
		{"BIT 4, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_4, &CPU.REG.A);}}, //0xCB67
		{"BIT 5, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.B);}}, //0xCB68
		{"BIT 5, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.C);}}, //0xCB69
		{"BIT 5, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.D);}}, //0xCB6A
		{"BIT 5, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.E);}}, //0xCB6B
		{"BIT 5, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.H);}}, //0xCB6C
		{"BIT 5, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.L);}}, //0xCB6D 
		{"BIT 5, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_5);}}, //0xCB6E
		{"BIT 5, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_5, &CPU.REG.A);}}, //0xCB6F

		{"BIT 6, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.B); }}, //0xCB70
		{"BIT 6, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.C);}}, //0xCB71
		{"BIT 6, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.D);}}, //0xCB72
		{"BIT 6, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.E);}}, //0xCB73
		{"BIT 6, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.H);}}, //0xCB74
		{"BIT 6, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.L);}}, //0xCB75
		{"BIT 6, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_6);}}, //0xCB76
		{"BIT 6, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_6, &CPU.REG.A);}}, //0xCB77
		{"BIT 7, B", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.B);}}, //0xCB78
		{"BIT 7, C", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.C);}}, //0xCB79
		{"BIT 7, D", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.D);}}, //0xCB7A
		{"BIT 7, E", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.E);}}, //0xCB7B
		{"BIT 7, H", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.H);}}, //0xCB7C
		{"BIT 7, L", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.L);}}, //0xCB7D 
		{"BIT 7, (HL)", 0, [](Cpu &CPU){ CPU.bit_i_atHL(BIT_7);}}, //0xCB7E
		{"BIT 7, A", 0, [](Cpu &CPU){ CPU.bit_i_rb(BIT_7, &CPU.REG.A);}}, //0xCB7F

		{"RES 0, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.B); }}, //0xCB80
		{"RES 0, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.C);}}, //0xCB81
		{"RES 0, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.D);}}, //0xCB82
		{"RES 0, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.E);}}, //0xCB83
		{"RES 0, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.H);}}, //0xCB84
		{"RES 0, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.L);}}, //0xCB85
		{"RES 0, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_0);}}, //0xCB86
		{"RES 0, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_0, &CPU.REG.A);}}, //0xCB87
		{"RES 1, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.B);}}, //0xCB88
		{"RES 1, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.C);}}, //0xCB89
		{"RES 1, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.D);}}, //0xCB8A
		{"RES 1, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.E);}}, //0xCB8B
		{"RES 1, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.H);}}, //0xCB8C
		{"RES 1, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.L);}}, //0xCB8D 
		{"RES 1, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_1);}}, //0xCB8E
		{"RES 1, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_1, &CPU.REG.A);}}, //0xCB8F

		{"RES 2, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.B); }}, //0xCB90
		{"RES 2, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.C);}}, //0xCB91
		{"RES 2, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.D);}}, //0xCB92
		{"RES 2, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.E);}}, //0xCB93
		{"RES 2, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.H);}}, //0xCB94
		{"RES 2, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.L);}}, //0xCB95
		{"RES 2, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_2);}}, //0xCB96
		{"RES 2, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_2, &CPU.REG.A);}}, //0xCB97
		{"RES 3, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.B);}}, //0xCB98
		{"RES 3, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.C);}}, //0xCB99
		{"RES 3, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.D);}}, //0xCB9A
		{"RES 3, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.E);}}, //0xCB9B
		{"RES 3, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.H);}}, //0xCB9C
		{"RES 3, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.L);}}, //0xCB9D 
		{"RES 3, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_3);}}, //0xCB9E
		{"RES 3, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_3, &CPU.REG.A);}}, //0xCB9F

		{"RES 4, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.B); }}, //0xCB61
		{"RES 4, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.C);}}, //0xCB62
		{"RES 4, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.D);}}, //0xCB63
		{"RES 4, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.E);}}, //0xCB64
		{"RES 4, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.H);}}, //0xCB65
		{"RES 4, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.L);}}, //0xCB66
		{"RES 4, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_4);}}, //0xCB67
		{"RES 4, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_4, &CPU.REG.A);}}, //0xCB68
		{"RES 5, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.B);}}, //0xCB69
		{"RES 5, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.C);}}, //0xCB6A
		{"RES 5, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.D);}}, //0xCB6B
		{"RES 5, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.E);}}, //0xCB6C
		{"RES 5, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.H);}}, //0xCB6D
		{"RES 5, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.L);}}, //0xCB6E 
		{"RES 5, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_5);}}, //0xCB6F
		{"RES 5, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_5, &CPU.REG.A);}}, //0xCB61

		{"RES 6, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.B); }}, //0xCB71
		{"RES 6, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.C);}}, //0xCB72
		{"RES 6, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.D);}}, //0xCB73
		{"RES 6, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.E);}}, //0xCB74
		{"RES 6, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.H);}}, //0xCB75
		{"RES 6, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.L);}}, //0xCB76
		{"RES 6, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_6);}}, //0xCB77
		{"RES 6, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_6, &CPU.REG.A);}}, //0xCB78
		{"RES 7, B", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.B);}}, //0xCB79
		{"RES 7, C", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.C);}}, //0xCB7A
		{"RES 7, D", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.D);}}, //0xCB7B
		{"RES 7, E", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.E);}}, //0xCB7C
		{"RES 7, H", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.H);}}, //0xCB7D
		{"RES 7, L", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.L);}}, //0xCB7E 
		{"RES 7, (HL)", 0, [](Cpu &CPU){ CPU.res_i_atHL(BIT_7);}}, //0xCB7F
		{"RES 7, A", 0, [](Cpu &CPU){ CPU.res_i_rb(BIT_7, &CPU.REG.A);}}, //0xCB71	

		{"SET 0, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.B); }}, //0xCB41
		{"SET 0, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.C);}}, //0xCB42
		{"SET 0, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.D);}}, //0xCB43
		{"SET 0, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.E);}}, //0xCB44
		{"SET 0, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.H);}}, //0xCB45
		{"SET 0, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.L);}}, //0xCB46
		{"SET 0, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_0);}}, //0xCB47
		{"SET 0, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_0, &CPU.REG.A);}}, //0xCB48
		{"SET 1, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.B);}}, //0xCB49
		{"SET 1, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.C);}}, //0xCB4A
		{"SET 1, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.D);}}, //0xCB4B
		{"SET 1, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.E);}}, //0xCB4C
		{"SET 1, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.H);}}, //0xCB4D
		{"SET 1, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.L);}}, //0xCB4E 
		{"SET 1, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_1);}}, //0xCB4F
		{"SET 1, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_1, &CPU.REG.A);}}, //0xCB41

		{"SET 2, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.B); }}, //0xCB51
		{"SET 2, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.C);}}, //0xCB52
		{"SET 2, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.D);}}, //0xCB53
		{"SET 2, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.E);}}, //0xCB54
		{"SET 2, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.H);}}, //0xCB55
		{"SET 2, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.L);}}, //0xCB56
		{"SET 2, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_2);}}, //0xCB57
		{"SET 2, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_2, &CPU.REG.A);}}, //0xCB58
		{"SET 3, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.B);}}, //0xCB59
		{"SET 3, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.C);}}, //0xCB5A
		{"SET 3, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.D);}}, //0xCB5B
		{"SET 3, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.E);}}, //0xCB5C
		{"SET 3, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.H);}}, //0xCB5D
		{"SET 3, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.L);}}, //0xCB5E 
		{"SET 3, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_3);}}, //0xCB5F
		{"SET 3, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_3, &CPU.REG.A);}}, //0xCB51

		{"SET 4, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.B); }}, //0xCB61
		{"SET 4, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.C);}}, //0xCB62
		{"SET 4, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.D);}}, //0xCB63
		{"SET 4, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.E);}}, //0xCB64
		{"SET 4, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.H);}}, //0xCB65
		{"SET 4, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.L);}}, //0xCB66
		{"SET 4, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_4);}}, //0xCB67
		{"SET 4, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_4, &CPU.REG.A);}}, //0xCB68
		{"SET 5, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.B);}}, //0xCB69
		{"SET 5, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.C);}}, //0xCB6A
		{"SET 5, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.D);}}, //0xCB6B
		{"SET 5, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.E);}}, //0xCB6C
		{"SET 5, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.H);}}, //0xCB6D
		{"SET 5, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.L);}}, //0xCB6E 
		{"SET 5, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_5);}}, //0xCB6F
		{"SET 5, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_5, &CPU.REG.A);}}, //0xCB61

		{"SET 6, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.B); }}, //0xCB71
		{"SET 6, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.C);}}, //0xCB72
		{"SET 6, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.D);}}, //0xCB73
		{"SET 6, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.E);}}, //0xCB74
		{"SET 6, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.H);}}, //0xCB75
		{"SET 6, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.L);}}, //0xCB76
		{"SET 6, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_6);}}, //0xCB77
		{"SET 6, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_6, &CPU.REG.A);}}, //0xCB78
		{"SET 7, B", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.B);}}, //0xCB79
		{"SET 7, C", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.C);}}, //0xCB7A
		{"SET 7, D", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.D);}}, //0xCB7B
		{"SET 7, E", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.E);}}, //0xCB7C
		{"SET 7, H", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.H);}}, //0xCB7D
		{"SET 7, L", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.L);}}, //0xCB7E 
		{"SET 7, (HL)", 0, [](Cpu &CPU){ CPU.set_i_atHL(BIT_7);}}, //0xCB7F
		{"SET 7, A", 0, [](Cpu &CPU){ CPU.set_i_rb(BIT_7, &CPU.REG.A);}} //0xCB71	
	};

	void ext() {
		uint8_t opcode = argbyte();
		ext_instructions[opcode].fn(*this);
	}

	Instruction instructions[256] = {
		{"NOP",             0, [](Cpu &CPU){ CPU.nop(); }},                  // 0x00
		{"LD BC, 0x%04X",   2, [](Cpu &CPU){ CPU.ld_rw_nn(&CPU.REG.BC); }},  // 0x01
		{"LD (BC), A",      0, [](Cpu &CPU){ CPU.ld_atrw_A(&CPU.REG.BC); }}, // 0x02
		{"INC BC",          0, [](Cpu &CPU){ CPU.inc_rw(&CPU.REG.BC); }},    // 0x03
		{"INC B",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.B); }},     // 0x04
		{"DEC B",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.B); }},     // 0x05
		{"LD B, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.B); }},    // 0x06
		{"RLCA",            0, [](Cpu &CPU){ CPU.rlca(); }},                 // 0x07
		{"LD (0x%04X), SP", 2, [](Cpu &CPU){ CPU.ld_atnn_SP(); }},           // 0x08
		{"ADD HL, BC",      0, [](Cpu &CPU){ CPU.add_hl_rw(&CPU.REG.BC); }}, // 0x09
		{"LD A, (BC)",      0, [](Cpu &CPU){ CPU.ld_A_atrw(&CPU.REG.BC); }}, // 0x0A
		{"DEC BC",          0, [](Cpu &CPU){ CPU.dec_rw(&CPU.REG.BC); }},    // 0x0B
		{"INC C",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.C); }},     // 0x0C
		{"DEC C",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.C); }},     // 0x0D
		{"LD C, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.C); }},    // 0x0E
		{"RRCA",            0, [](Cpu &CPU){ CPU.rrca(); }},                 // 0x0F

		{"STOP",            0, [](Cpu &CPU){ CPU.TODO(); }},                 // 0x10
		{"LD DE, 0x%04X",   2, [](Cpu &CPU){ CPU.ld_rw_nn(&CPU.REG.DE); }},  // 0x11
		{"LD (DE), A",      0, [](Cpu &CPU){ CPU.ld_atrw_A(&CPU.REG.DE); }}, // 0x12
		{"INC DE",          0, [](Cpu &CPU){ CPU.inc_rw(&CPU.REG.DE); } },   // 0x13
		{"INC D",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.D); }},     // 0x14
		{"DEC D",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.D); }},     // 0x15
		{"LD D, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.D); }},    // 0x16
		{"RLA",             0, [](Cpu &CPU){ CPU.rla(); }},                  // 0x17
		{"JR 0x%02X",       1, [](Cpu &CPU){ CPU.jr_e(); }},                 // 0x18
		{"ADD HL, DE",      0, [](Cpu &CPU){ CPU.add_hl_rw(&CPU.REG.DE); }}, // 0x19
		{"LD A, (DE)",      0, [](Cpu &CPU){ CPU.ld_A_atrw(&CPU.REG.DE); }}, // 0x1A
		{"DEC DE",          0, [](Cpu &CPU){ CPU.dec_rw(&CPU.REG.DE); }},    // 0x1B
		{"INC E",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.E); }},     // 0x1C
		{"DEC E",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.E); }},     // 0x1D
		{"LD E, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.E); }},    // 0x1E
		{"RRA",             0, [](Cpu &CPU){ CPU.rra(); }},                  // 0x1F

		{"JR NZ, 0x%02X",   1, [](Cpu &CPU){ CPU.jr_nf_e(FLAG_Z); }},        // 0x20
		{"LD HL, 0x%04X",   2, [](Cpu &CPU){ CPU.ld_rw_nn(&CPU.REG.HL); }},  // 0x21
		{"LDI (HL), A",     0, [](Cpu &CPU){ CPU.ldi_atHL_A(); }},           // 0x22
		{"INC HL",          0, [](Cpu &CPU){ CPU.inc_rw(&CPU.REG.HL); }},    // 0x23
		{"INC H",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.H); }},     // 0x24
		{"DEC H",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.H); }},     // 0x25
		{"LD H, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.H); }},    // 0x26
		{"DAA",             0, [](Cpu &CPU){ CPU.daa(); }},                  // 0x27
		{"JR Z, 0x%02X",    1, [](Cpu &CPU){ CPU.jr_f_e(FLAG_Z); }},         // 0x28
		{"ADD HL, HL",      0, [](Cpu &CPU){ CPU.add_hl_rw(&CPU.REG.HL); }}, // 0x29
		{"LDI A, (HL)",     0, [](Cpu &CPU){ CPU.ldi_A_atHL(); }},           // 0x2A
		{"DEC HL",          0, [](Cpu &CPU){ CPU.dec_rw(&CPU.REG.HL); }},    // 0x2B
		{"INC L",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.L); }},     // 0x2C
		{"DEC L",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.L); }},     // 0x2D
		{"LD L, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.L); }},    // 0x2E
		{"CPL",             0, [](Cpu &CPU){ CPU.cpl(); }},                  // 0x2F
	 
		{"JR NC, 0x%02X",   1, [](Cpu &CPU){ CPU.jr_nf_e(FLAG_C); }},        // 0x30
		{"LD SP, 0x%04X",   2, [](Cpu &CPU){ CPU.ld_rw_nn(&CPU.REG.SP); }},  // 0x31
		{"LDD (HL), A",     0, [](Cpu &CPU){ CPU.ldd_atHL_A(); }},           // 0x32
		{"INC SP",          0, [](Cpu &CPU){ CPU.inc_rw(&CPU.REG.SP); }},    // 0x33
		{"INC (HL)",        0, [](Cpu &CPU){ CPU.inc_atHL(); }},             // 0x34
		{"DEC (HL)",        0, [](Cpu &CPU){ CPU.dec_atHL(); }},             // 0x35
		{"LD (HL), 0x%02X", 1, [](Cpu &CPU){ CPU.ld_atHL_n(); }},            // 0x36
		{"SCF",             0, [](Cpu &CPU){ CPU.scf(); }},                  // 0x37
		{"JR C, 0x%02X",    1, [](Cpu &CPU){ CPU.jr_f_e(FLAG_C); }},         // 0x38
		{"ADD HL, SP",      0, [](Cpu &CPU){ CPU.add_hl_rw(&CPU.REG.SP); }}, // 0x39
		{"LDD A, (HL)",     0, [](Cpu &CPU){ CPU.ldd_A_atHL(); }},           // 0x3A
		{"DEC SP",          0, [](Cpu &CPU){ CPU.dec_rw(&CPU.REG.SP); }},    // 0x3B
		{"INC A",           0, [](Cpu &CPU){ CPU.inc_rb(&CPU.REG.A); }},     // 0x3C
		{"DEC A",           0, [](Cpu &CPU){ CPU.dec_rb(&CPU.REG.A); }},     // 0x3D
		{"LD A, 0x%02X",    1, [](Cpu &CPU){ CPU.ld_rb_n(&CPU.REG.A); }},    // 0x3E
		{"CCF",             0, [](Cpu &CPU){ CPU.ccf(); }},                  // 0x3F

		{"LD B, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.B); }},       // 0x40
		{"LD B, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.C); }},       // 0x41
		{"LD B, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.D); }},       // 0x42
		{"LD B, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.E); }},       // 0x43
		{"LD B, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.H); }},       // 0x44
		{"LD B, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.L); }},       // 0x45
		{"LD B, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.B); }},              // 0x46
		{"LD B, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.A); }},       // 0x47
		{"LD C, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.B); }},       // 0x48
		{"LD C, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.C);}},        // 0x49
		{"LD C, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.D);}},        // 0x4A
		{"LD C, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.E);}},        // 0x4B
		{"LD C, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.H);}},        // 0x4C
		{"LD C, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.L);}},        // 0x4D
		{"LD C, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.C); }},              // 0x4E
		{"LD C, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.A);}},        // 0x4F

		{"LD D, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.B);}},        // 0x50
		{"LD D, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.C);}},        // 0x51
		{"LD D, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.D);}},        // 0x52
		{"LD D, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.E);}},        // 0x53
		{"LD D, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.H);}},        // 0x54
		{"LD D, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.L);}},        // 0x55
		{"LD D, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.D); }},              // 0x56
		{"LD D, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.A);}},        // 0x57
		{"LD E, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.B);}},        // 0x58
		{"LD E, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.C);}},        // 0x59
		{"LD E, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.D);}},        // 0x5A
		{"LD E, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.E);}},        // 0x5B
		{"LD E, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.H);}},        // 0x5C
		{"LD E, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.L);}},        // 0x5D
		{"LD E, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.E); }},              // 0x5E
		{"LD E, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.A);}},        // 0x5F

		{"LD H, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.B);}},        // 0x60
		{"LD H, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.C);}},        // 0x61
		{"LD H, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.D);}},        // 0x62
		{"LD H, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.E);}},        // 0x63
		{"LD H, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.H);}},        // 0x64
		{"LD H, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.L);}},        // 0x65
		{"LD H, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.H); }},              // 0x66
		{"LD H, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.A);}},        // 0x67
		{"LD L, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.B);}},        // 0x68
		{"LD L, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.C);}},        // 0x69
		{"LD L, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.D);}},        // 0x6A
		{"LD L, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.E);}},        // 0x6B
		{"LD L, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.H);}},        // 0x6C
		{"LD L, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.L);}},        // 0x6D
		{"LD L, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.L); }},              // 0x6E
		{"LD L, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.A);}},        // 0x6F

		{"LD (HL), B", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.B); }},     // 0x70
		{"LD (HL), C", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.C); }},     // 0x71
		{"LD (HL), D", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.D); }},     // 0x72
		{"LD (HL), E", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.E); }},     // 0x73
		{"LD (HL), H", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.H); }},     // 0x74
		{"LD (HL), L", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.L); }},     // 0x75
		{"HALT", 0, [](Cpu &CPU){ CPU.halt(); }},                           // 0x76
		{"LD (HL), A", 0, [](Cpu &CPU){ CPU.ld_atHL_rb(&CPU.REG.A); }},              // 0x77
		{"LD A, B", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.B);}},        // 0x78
		{"LD A, C", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.C);}},        // 0x79
		{"LD A, D", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.D);}},        // 0x7A
		{"LD A, E", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.E);}},        // 0x7B
		{"LD A, H", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.H);}},        // 0x7C
		{"LD A, L", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.L);}},        // 0x7D
		{"LD A, (HL)", 0, [](Cpu &CPU){ CPU.ld_rb_atHL(&CPU.REG.A); }},              // 0x7E
		{"LD A, A", 0, [](Cpu &CPU){ CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.A);}},        // 0x7F

		{"ADD A, B", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.B); }},       // 0x80
		{"ADD A, C", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.C); }},       // 0x81
		{"ADD A, D", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.D); }},       // 0x82
		{"ADD A, E", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.E); }},       // 0x83
		{"ADD A, H", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.H); }},       // 0x84
		{"ADD A, L", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.L); }},       // 0x85
		{"ADD A, (HL)", 0, [](Cpu &CPU){ CPU.add_A_atHL(); }},            // 0x86
		{"ADD A, A", 0, [](Cpu &CPU){ CPU.add_A_rb(&CPU.REG.A); }},       // 0x87
		{"ADC A, B", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.B); }},       // 0x88
		{"ADC A, C", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.C); }},       // 0x89
		{"ADC A, D", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.D); }},       // 0x8A
		{"ADC A, E", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.E); }},       // 0x8B
		{"ADC A, H", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.H); }},       // 0x8C
		{"ADC A, L", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.L); }},       // 0x8D
		{"ADC A, (HL)", 0, [](Cpu &CPU){ CPU.adc_A_atHL(); }},            // 0x8E
		{"ADC A, A", 0, [](Cpu &CPU){ CPU.adc_A_rb(&CPU.REG.A); }},       // 0x8F

		{"SUB A, B", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.B); }},       // 0x90
		{"SUB A, C", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.C); }},       // 0x91
		{"SUB A, D", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.D); }},       // 0x92
		{"SUB A, E", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.E); }},       // 0x93
		{"SUB A, H", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.H); }},       // 0x94
		{"SUB A, L", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.L); }},       // 0x95
		{"SUB A, (HL)", 0, [](Cpu &CPU){ CPU.sub_A_atHL(); }},            // 0x96
		{"SUB A, A", 0, [](Cpu &CPU){ CPU.sub_A_rb(&CPU.REG.A); }},       // 0x97
		{"SBC A, B", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.B); }},       // 0x98
		{"SBC A, C", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.C); }},       // 0x99
		{"SBC A, D", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.D); }},       // 0x9A
		{"SBC A, E", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.E); }},       // 0x9B
		{"SBC A, H", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.H); }},       // 0x9C
		{"SBC A, L", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.L); }},       // 0x9D
		{"SBC A, (HL)", 0, [](Cpu &CPU){ CPU.sbc_A_atHL(); }},            // 0x9E
		{"SBC A, A", 0, [](Cpu &CPU){ CPU.sbc_A_rb(&CPU.REG.A); }},       // 0x9F

		{"AND B", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.B); }},         // 0xA0
		{"AND C", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.C); }},         // 0xA1
		{"AND D", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.D); }},         // 0xA2
		{"AND E", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.E); }},         // 0xA3
		{"AND H", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.H); }},         // 0xA4
		{"AND L", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.L); }},         // 0xA5
		{"AND (HL)", 0, [](Cpu &CPU){ CPU.and_atHL(); }},              // 0xA6
		{"AND A", 0, [](Cpu &CPU){ CPU.and_rb(&CPU.REG.A); }},         // 0xA7
		{"XOR B", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.B); }},         // 0xA8
		{"XOR C", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.C);}},          // 0xA9
		{"XOR D", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.D);}},          // 0xAA
		{"XOR E", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.E);}},          // 0xAB
		{"XOR H", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.H);}},          // 0xAC
		{"XOR L", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.L);}},          // 0xAD
		{"XOR (HL)", 0, [](Cpu &CPU){ CPU.xor_atHL();}},               // 0xAE
		{"XOR A", 0, [](Cpu &CPU){ CPU.xor_rb(&CPU.REG.A);}},          // 0xAF

		{"OR B", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.B); }},           // 0xB0
		{"OR C", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.C); }},           // 0xB1
		{"OR D", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.D); }},           // 0xB2
		{"OR E", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.E); }},           // 0xB3
		{"OR H", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.H); }},           // 0xB4
		{"OR L", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.L); }},           // 0xB5
		{"OR (HL)", 0, [](Cpu &CPU){ CPU.or_atHL(); }},                // 0xB6
		{"OR A", 0, [](Cpu &CPU){ CPU.or_rb(&CPU.REG.A); }},           // 0xB7
		{"CP B", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.B); }},           // 0xB8
		{"CP C", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.C); }},           // 0xB9
		{"CP D", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.D); }},           // 0xBA
		{"CP E", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.E); }},           // 0xBB
		{"CP H", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.H); }},           // 0xBC
		{"CP L", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.L); }},           // 0xBD
		{"CP (HL)", 0, [](Cpu &CPU){ CPU.cp_atHL(); }},                // 0xBE
		{"CP A", 0, [](Cpu &CPU){ CPU.cp_rb(&CPU.REG.A); }},           // 0xBF

		{"RET NZ",          0, [](Cpu &CPU){ CPU.ret_nf(FLAG_Z); }},       // 0xC0
		{"POP BC",          0, [](Cpu &CPU){ CPU.pop_rw(&CPU.REG.BC); }},  // 0xC1
		{"JP NZ, 0x%04X",   2, [](Cpu &CPU){ CPU.jp_nf_nn(FLAG_Z); }},     // 0xC2
		{"JP 0x%04X",       2, [](Cpu &CPU){ CPU.jp_nn(); }},              // 0xC3
		{"CALL NZ, 0x%04X", 2, [](Cpu &CPU){ CPU.call_nf_nn(FLAG_Z); }},   // 0xC4
		{"PUSH BC",         0, [](Cpu &CPU){ CPU.push_rw(&CPU.REG.BC); }}, // 0xC5
		{"ADD A, 0x%02X",   1, [](Cpu &CPU){ CPU.add_A_n(); }},            // 0xC6
		{"RST 0",           0, [](Cpu &CPU){ CPU.rst(0x00); }},            // 0xC7
		{"RET Z",           0, [](Cpu &CPU){ CPU.ret_f(FLAG_Z); }},        // 0xC8
		{"RET",             0, [](Cpu &CPU){ CPU.ret(); }},                // 0xC9
		{"JP Z, 0x%04X",    2, [](Cpu &CPU){ CPU.jp_f_nn(FLAG_Z); }},      // 0xCA
		{"Ext Op",          1, [](Cpu &CPU){ CPU.ext(); }},                // 0xCB
		{"CALL Z, 0x%04X",  2, [](Cpu &CPU){ CPU.call_f_nn(FLAG_Z); }},    // 0xCC
		{"CALL 0x%04X",     2, [](Cpu &CPU){ CPU.call_nn(); }},            // 0xCD
		{"ADC A, 0x%02X",   1, [](Cpu &CPU){ CPU.adc_A_n(); }},            // 0xCE
		{"RST 8",           0, [](Cpu &CPU){ CPU.rst(0x08); }},            // 0xCF

		{"RET NC",          0, [](Cpu &CPU){ CPU.ret_nf(FLAG_C); }},       // 0xD0
		{"POP DE",          0, [](Cpu &CPU){ CPU.pop_rw(&CPU.REG.DE); }},  // 0xD1
		{"JP NC, 0x%04X",   2, [](Cpu &CPU){ CPU.jp_nf_nn(FLAG_C); }},     // 0xD2
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xD3
		{"CALL NC, 0x%04X", 2, [](Cpu &CPU){ CPU.call_nf_nn(FLAG_C); }},   // 0xD4
		{"PUSH DE",         0, [](Cpu &CPU){ CPU.push_rw(&CPU.REG.DE); }}, // 0xD5
		{"SUB A, 0x%02X",   1, [](Cpu &CPU){ CPU.sub_A_n(); }},            // 0xD6
		{"RST 10",          0, [](Cpu &CPU){ CPU.rst(0x10); }},            // 0xD7
		{"RET C",           0, [](Cpu &CPU){ CPU.ret_f(FLAG_C); }},        // 0xD8
		{"RETI",            0, [](Cpu &CPU){ CPU.reti(); }},               // 0xD9
		{"JP C, 0x%04X",    2, [](Cpu &CPU){ CPU.jp_f_nn(FLAG_C); }},      // 0xDA
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xDB
		{"CALL C, 0x%04X",  2, [](Cpu &CPU){ CPU.call_f_nn(FLAG_C); }},    // 0xDC
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xDD
		{"SBC A, 0x%02X",   1, [](Cpu &CPU){ CPU.sbc_A_n(); }},            // 0xDE
		{"RST 18",          0, [](Cpu &CPU){ CPU.rst(0x18); }},            // 0xDF

		{"LDH (0x%02X), A", 1, [](Cpu &CPU){ CPU.ldh_atn_A(); }},          // 0xE0
		{"POP HL",          0, [](Cpu &CPU){ CPU.pop_rw(&CPU.REG.HL); }},  // 0xE1
		{"LDH (C), A",      0, [](Cpu &CPU){ CPU.ldh_atC_A(); } },         // 0xE2
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xE3
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xE4
		{"PUSH HL",         0, [](Cpu &CPU){ CPU.push_rw(&CPU.REG.HL); }}, // 0xE5
		{"AND 0x%02X",      1, [](Cpu &CPU){ CPU.and_n(); }},              // 0xE6
		{"RST 20",          0, [](Cpu &CPU){ CPU.rst(0x20); }},            // 0xE7
		{"ADD SP, 0x%02X",  0, [](Cpu &CPU){ CPU.add_SP_e(); }},           // 0xE8
		{"JP (HL)",         0, [](Cpu &CPU){ CPU.jp_atHL(); }},            // 0xE9
		{"LD (0x%04X), A",  2, [](Cpu &CPU){ CPU.ld_atnn_A(); }},          // 0xEA
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xEB
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xEC
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xED
		{"XOR 0x%02X",      1, [](Cpu &CPU){ CPU.xor_n(); }},              // 0xEE
		{"RST 28",          0, [](Cpu &CPU){ CPU.rst(0x28); }},            // 0xEF

		{"LDH A, (0x%02X)", 1, [](Cpu &CPU){ CPU.ldh_A_atn(); }},          // 0xF0
		{"POP AF",          0, [](Cpu &CPU){ CPU.pop_AF(); }},             // 0xF1
		{"LDH A, (C)",      0, [](Cpu &CPU){ CPU.ldh_A_atC(); }},          // 0xF2
		{"DI",              0, [](Cpu &CPU){ CPU.di(); }},                 // 0xF3
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xF4
		{"PUSH AF",         0, [](Cpu &CPU){ CPU.push_rw(&CPU.REG.AF); }}, // 0xF5
		{"OR 0x%02X",       1, [](Cpu &CPU){ CPU.or_n(); }},               // 0xF6
		{"RST 30",          0, [](Cpu &CPU){ CPU.rst(0x30); }},            // 0xF7
		{"LDHL SP, 0x%02X", 0, [](Cpu &CPU){ CPU.ld_HL_SP_e(); }},         // 0xF8
		{"LD SP, HL",       0, [](Cpu &CPU){ CPU.ld_SP_HL(); }},           // 0xF9
		{"LD A, (0x%04X)",  2, [](Cpu &CPU){ CPU.ld_A_atnn(); }},          // 0xFA
		{"EI",              0, [](Cpu &CPU){ CPU.ei(); }},                 // 0xFB
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xFC
		{"XX",              0, [](Cpu &CPU){ CPU.TODO(); }},               // 0xFD
		{"CP 0x%02X",       1, [](Cpu &CPU){ CPU.cp_n(); }},               // 0xFE
		{"RST 38",          0, [](Cpu &CPU){ CPU.rst(0x38); }}             // 0xFF
	};

};