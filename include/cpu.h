#pragma once
#include <algorithm>
#include <functional>
#include <inttypes.h>

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
        void (*fn)(Cpu &);
    } Instruction;

    Cpu(Memory &MemRef, Registers &RegRef) : MEM(MemRef), REG(RegRef) {
        init_instructions();
        init_ext_instructions();
    }

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

    Instruction instructions[256];
    Instruction ext_instructions[256];

    bool is_stuck() {
        return stuck_flag;
    }

  private:
    bool stuck_flag = false;
    void init_instructions();
    void init_ext_instructions();

    void nop() {
        REG.TCLK = 4;
    }

    // 8-bit arithmetic

    void inc_rb(uint8_t *ptr) {
        (*ptr)++;

        REG.FLAG_N = 0;
        REG.FLAG_H = ((*ptr & 0x0F) == 0);
        REG.FLAG_Z = (*ptr == 0);
        REG.TCLK   = 4;
    }

    void inc_atHL() {
        uint8_t val = readByte(REG.HL);
        writeByte(REG.HL, val + 1);

        REG.FLAG_N = 0;
        REG.FLAG_H = ((val & 0x0F) == 0x0F);
        REG.FLAG_Z = ((val == 0xFF));

        REG.TCLK = 12;
    }

    void dec_rb(uint8_t *ptr) {
        (*ptr)--;

        REG.FLAG_N = 1;
        REG.FLAG_H = ((*ptr & 0x0F) == 0x0F);
        REG.FLAG_Z = (*ptr == 0);
        REG.TCLK   = 4;
    }

    void dec_atHL() {
        uint8_t val = readByte(REG.HL);
        writeByte(REG.HL, val - 1);

        REG.FLAG_N = 1;
        REG.FLAG_H = ((val & 0x0F) == 0x00);
        REG.FLAG_Z = ((val - 1) == 0);

        REG.TCLK = 12;
    }

    void xor_rb(uint8_t *from) {
        REG.A ^= *from;

        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void xor_n() {
        REG.A ^= argbyte();

        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void xor_atHL() {
        REG.A ^= readByte(REG.HL);

        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void or_rb(uint8_t *from) {
        REG.A |= *from;

        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void or_n() {
        REG.A |= argbyte();

        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void or_atHL() {
        REG.A |= readByte(REG.HL);

        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void and_rb(uint8_t *from) {
        REG.A &= *from;

        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_H = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void and_n() {
        REG.A &= argbyte();

        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_H = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void and_atHL() {
        REG.A &= readByte(REG.HL);

        REG.FLAG_N = 0;
        REG.FLAG_C = 0;
        REG.FLAG_H = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void cp_rb(uint8_t *from) {

        uint8_t e = *from;

        REG.FLAG_N = 1;
        REG.FLAG_Z = (e == REG.A);
        REG.FLAG_C = (e > REG.A);
        REG.FLAG_H = ((e & 0x0F) > (REG.A & 0x0F));

        REG.TCLK = 4;
    }

    void cp_n() {

        uint8_t e = argbyte();

        REG.FLAG_N = 1;
        REG.FLAG_Z = (e == REG.A);
        REG.FLAG_C = (e > REG.A);
        REG.FLAG_H = ((e & 0x0F) > (REG.A & 0x0F));

        REG.TCLK = 8;
    }

    void cp_atHL() {

        uint8_t e = readByte(REG.HL);

        REG.FLAG_N = 1;
        REG.FLAG_Z = (e == REG.A);
        REG.FLAG_C = (e > REG.A);
        REG.FLAG_H = ((e & 0x0F) > (REG.A & 0x0F));

        REG.TCLK = 8;
    }

    void add_A_rb(uint8_t *ptr) {
        REG.FLAG_C = (0xFF - REG.A < *ptr);
        REG.FLAG_H = (((REG.A & 0x0F) + (*ptr & 0x0F)) & 0xF0);
        REG.A += *ptr;
        REG.FLAG_N = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void add_A_n() {
        uint8_t val = argbyte();
        REG.FLAG_C  = (0xFF - REG.A < val);
        REG.FLAG_H  = (((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
        REG.A += val;
        REG.FLAG_N = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void add_A_atHL() {
        uint8_t val = readByte(REG.HL);

        REG.FLAG_C = (0xFF - REG.A < val);
        REG.FLAG_H = (((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
        REG.A += val;
        REG.FLAG_N = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void add_SP_e() {
        uint8_t n = argbyte();
        int8_t e  = *reinterpret_cast<uint8_t *>(&n);

        uint8_t lb = REG.SP & 0x00FF;

        REG.FLAG_C = ((0xFF - lb) < n);
        REG.FLAG_H = (((lb & 0x0F) + (n & 0x0F)) & 0x10);
        REG.SP += e;

        REG.FLAG_Z = 0;
        REG.FLAG_N = 0;

        REG.TCLK = 16;
    }

    void adc_A_rb(uint8_t *ptr) {
        bool carry = REG.FLAG_C;
        REG.FLAG_C = (0xFF - REG.A < *ptr);
        REG.FLAG_H = (((REG.A & 0x0F) + (*ptr & 0x0F)) & 0xF0);
        REG.A += *ptr;
        REG.FLAG_C = (REG.FLAG_C || (0xFF - REG.A < carry));
        REG.FLAG_H = (REG.FLAG_H || (((REG.A & 0x0F) + carry) & 0xF0));
        REG.A += carry;
        REG.FLAG_N = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void adc_A_n() {
        uint8_t val = argbyte();
        bool carry  = REG.FLAG_C;
        REG.FLAG_C  = (0xFF - REG.A < val);
        REG.FLAG_H  = (((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
        REG.A += val;
        REG.FLAG_C = (REG.FLAG_C || (0xFF - REG.A < carry));
        REG.FLAG_H = (REG.FLAG_H || (((REG.A & 0x0F) + carry) & 0xF0));
        REG.A += carry;
        REG.FLAG_N = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void adc_A_atHL() {
        uint8_t val = readByte(REG.HL);
        bool carry  = REG.FLAG_C;
        REG.FLAG_C  = (0xFF - REG.A < val);
        REG.FLAG_H  = (((REG.A & 0x0F) + (val & 0x0F)) & 0xF0);
        REG.A += val;
        REG.FLAG_C = (REG.FLAG_C || (0xFF - REG.A < carry));
        REG.FLAG_H = (REG.FLAG_H || (((REG.A & 0x0F) + carry) & 0xF0));
        REG.A += carry;
        REG.FLAG_N = 0;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void sub_A_rb(uint8_t *ptr) {
        REG.FLAG_C = (REG.A < *ptr);
        REG.FLAG_H = ((REG.A & 0x0F) < (*ptr & 0x0F));
        REG.A -= *ptr;
        REG.FLAG_N = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void sub_A_n() {
        uint8_t val = argbyte();
        REG.FLAG_C  = (REG.A < val);
        REG.FLAG_H  = ((REG.A & 0x0F) < (val & 0x0F));
        REG.A -= val;
        REG.FLAG_N = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void sub_A_atHL() {
        uint8_t val = readByte(REG.HL);
        REG.FLAG_C  = (REG.A < val);
        REG.FLAG_H  = ((REG.A & 0x0F) < (val & 0x0F));
        REG.A -= val;
        REG.FLAG_N = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void sbc_A_rb(uint8_t *ptr) {
        bool carry = REG.FLAG_C;
        REG.FLAG_C = (REG.A < *ptr);
        REG.FLAG_H = ((REG.A & 0x0F) < (*ptr & 0x0F));
        REG.A -= *ptr;
        REG.FLAG_C = (REG.FLAG_C || (REG.A < carry));
        REG.FLAG_H = (REG.FLAG_H || (((REG.A & 0x0F) < carry)));
        REG.A -= carry;
        REG.FLAG_N = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 4;
    }

    void sbc_A_n() {
        bool carry  = REG.FLAG_C;
        uint8_t val = argbyte();
        REG.FLAG_C  = (REG.A < val);
        REG.FLAG_H  = ((REG.A & 0x0F) < (val & 0x0F));
        REG.A -= val;
        REG.FLAG_C = (REG.FLAG_C || (REG.A < carry));
        REG.FLAG_H = (REG.FLAG_H || (((REG.A & 0x0F) < carry)));
        REG.A -= carry;
        REG.FLAG_N = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    void sbc_A_atHL() {
        bool carry  = REG.FLAG_C;
        uint8_t val = readByte(REG.HL);
        REG.FLAG_C  = (REG.A < val);
        REG.FLAG_H  = ((REG.A & 0x0F) < (val & 0x0F));
        REG.A -= val;
        REG.FLAG_C = (REG.FLAG_C || (REG.A < carry));
        REG.FLAG_H = (REG.FLAG_H || (((REG.A & 0x0F) < carry)));
        REG.A -= carry;
        REG.FLAG_N = 1;
        REG.FLAG_Z = (REG.A == 0);

        REG.TCLK = 8;
    }

    // 16-bit arithmetic

    void inc_rw(uint16_t *ptr) {
        (*ptr)++;
        REG.TCLK = 8;
    }

    void dec_rw(uint16_t *ptr) {
        (*ptr)--;
        REG.TCLK = 8;
    }

    void add_hl_rw(uint16_t *ptr) {
        REG.FLAG_C = (0xFFFF - REG.HL < *ptr);
        REG.FLAG_H = (((REG.HL & 0x0FFF) + (*ptr & 0x0FFF)) & 0x1000);
        REG.HL += *ptr;
        REG.FLAG_N = 0;

        REG.TCLK = 8;
    }

    // 8-bit loads

    void ld_rb_rb(uint8_t *to, uint8_t *from) {
        (*to)    = (*from);
        REG.TCLK = 4;
    }

    void ld_rb_n(uint8_t *to) {
        (*to)    = argbyte();
        REG.TCLK = 8;
    }

    void ld_rb_atHL(uint8_t *to) {
        (*to)    = readByte(REG.HL);
        REG.TCLK = 8;
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
        REG.A    = readWord(*addr);
        REG.TCLK = 8;
    }

    void ld_A_atnn() {
        REG.A    = readByte(argword());
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
        REG.A    = readByte(0xFF00 | REG.C);
        REG.TCLK = 8;
    }

    void ldh_atC_A() {
        writeByte(0xFF00 | REG.C, REG.A);
        REG.TCLK = 8;
    }

    void ldd_A_atHL() {
        REG.A    = readByte(REG.HL--);
        REG.TCLK = 8;
    }

    void ldd_atHL_A() {
        writeByte(REG.HL--, REG.A);
        REG.TCLK = 8;
    }

    void ldi_A_atHL() {
        REG.A    = readByte(REG.HL++);
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
        REG.A    = readByte(0xFF00 | argbyte());
        REG.TCLK = 12;
    }

    // 16-bit loads

    void ld_rw_nn(uint16_t *to) {
        *to      = argword();
        REG.TCLK = 12;
    }

    void ld_atnn_SP() {
        uint16_t addr = argword();
        writeWord(addr, REG.SP);
        REG.TCLK = 20;
    }

    void ld_SP_HL() {
        REG.SP   = REG.HL;
        REG.TCLK = 8;
    }

    void ld_HL_SP_e() {

        uint8_t n = argbyte();
        int8_t e  = *reinterpret_cast<uint8_t *>(&n);

        REG.HL = REG.SP + e;

        REG.FLAG_H = (0x000F - (REG.SP & 0x000F) < (e & 0x000F));
        REG.FLAG_C = (0x00FF - (REG.SP & 0x00FF) < (e & 0x00FF));
        REG.FLAG_Z = 0;
        REG.FLAG_N = 0;

        REG.TCLK = 12;
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
        REG.FLAG_C = (carry);
        REG.A >>= 1;
        if (carry)
            REG.A |= BIT_7;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = 0;

        REG.TCLK = 4;
    }

    void rrc_rb(uint8_t *at) {
        bool carry = *at & BIT_0;
        REG.FLAG_C = (carry);
        *at >>= 1;
        if (carry)
            *at |= BIT_7;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void rrc_atHL() {
        uint8_t val = readByte(REG.HL);
        bool carry  = val & BIT_0;
        REG.FLAG_C  = (carry);
        val >>= 1;
        if (carry)
            val |= BIT_7;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    // rotate left
    void rlca() {
        bool carry = REG.A & BIT_7;
        REG.FLAG_C = (carry);
        REG.A <<= 1;
        if (carry)
            REG.A |= BIT_0;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = 0;

        REG.TCLK = 4;
    }

    void rlc_rb(uint8_t *at) {
        bool carry = *at & BIT_7;
        REG.FLAG_C = (carry);
        *at <<= 1;
        if (carry)
            *at |= BIT_0;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void rlc_atHL() {
        uint8_t val = readByte(REG.HL);
        bool carry  = val & BIT_7;
        REG.FLAG_C  = (carry);
        val <<= 1;
        if (carry)
            val |= BIT_0;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    // rotate right (through carry flag)
    void rra() {
        bool carry = REG.FLAG_C;
        REG.FLAG_C = (REG.A & BIT_0);
        REG.A >>= 1;
        if (carry)
            REG.A |= BIT_7;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = 0;

        REG.TCLK = 4;
    }

    void rr_rb(uint8_t *at) {
        bool carry = REG.FLAG_C;
        REG.FLAG_C = (*at & BIT_0);
        *at >>= 1;
        if (carry)
            *at |= BIT_7;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void rr_atHL() {
        uint8_t val = readByte(REG.HL);
        bool carry  = REG.FLAG_C;
        REG.FLAG_C  = (val & BIT_0);
        val >>= 1;
        if (carry)
            val |= BIT_7;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    // rotate left (through carry flag)
    void rla() {
        bool carry = REG.FLAG_C;
        REG.FLAG_C = (REG.A & BIT_7);
        REG.A <<= 1;
        if (carry)
            REG.A |= BIT_0;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = 0;

        REG.TCLK = 4;
    }

    void rl_rb(uint8_t *at) {
        bool carry = REG.FLAG_C;
        REG.FLAG_C = (*at & BIT_7);
        *at <<= 1;
        if (carry)
            *at |= BIT_0;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void rl_atHL() {
        uint8_t val = readByte(REG.HL);
        bool carry  = REG.FLAG_C;
        REG.FLAG_C  = (val & BIT_7);
        val <<= 1;
        if (carry)
            val |= BIT_0;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    // shift left
    void sla_rb(uint8_t *at) {
        REG.FLAG_C = (*at & BIT_7);
        *at <<= 1;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void sla_atHL() {
        uint8_t val = readByte(REG.HL);
        REG.FLAG_C  = (val & BIT_7);
        val <<= 1;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    // shift right (keep bit 7)
    void sra_rb(uint8_t *at) {
        REG.FLAG_C = (*at & BIT_0);
        *at >>= 1;
        if (*at & BIT_6)
            *at |= BIT_7;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void sra_atHL() {
        uint8_t val = readByte(REG.HL);
        REG.FLAG_C  = (val & BIT_0);
        val >>= 1;
        if (val & BIT_6)
            val |= BIT_7;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    // shift right
    void srl_rb(uint8_t *at) {
        REG.FLAG_C = (*at & BIT_0);
        *at >>= 1;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void srl_atHL() {
        uint8_t val = readByte(REG.HL);
        REG.FLAG_C  = (val & BIT_0);
        val >>= 1;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    void swap_rb(uint8_t *at) {
        uint8_t tmp = *at << 4;
        *at >>= 4;
        *at |= tmp;

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (*at == 0);

        REG.TCLK = 8;
    }

    void swap_atHL() {
        uint8_t val = readByte(REG.HL);
        uint8_t tmp = val << 4;
        val >>= 4;
        val |= tmp;
        writeByte(REG.HL, val);

        REG.FLAG_N = 0;
        REG.FLAG_H = 0;
        REG.FLAG_C = 0;
        REG.FLAG_Z = (val == 0);

        REG.TCLK = 16;
    }

    void cpl() {
        REG.A      = ~REG.A;
        REG.FLAG_N = 1;
        REG.FLAG_H = 1;

        REG.TCLK = 4;
    }

    void bit_i_rb(const uint8_t mask, uint8_t *from) {
        REG.FLAG_N = 0;
        REG.FLAG_H = 1;
        REG.FLAG_Z = ((*from & mask) == 0);

        REG.TCLK = 8;
    }

    void bit_i_atHL(const uint8_t mask) {
        REG.FLAG_N = 0;
        REG.FLAG_H = 1;
        REG.FLAG_Z = ((readByte(REG.HL) & mask) == 0);

        REG.TCLK = 12;
    }

    void res_i_rb(const uint8_t mask, uint8_t *to) {
        *to &= ~mask;
        REG.TCLK = 8;
    }

    void res_i_atHL(const uint8_t mask) {
        uint8_t val = readByte(REG.HL);
        val &= ~mask;
        writeByte(REG.HL, val);

        REG.TCLK = 16;
    }

    void set_i_rb(const uint8_t mask, uint8_t *to) {
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
        REG.PC   = argword();
    }

    void jp_f_nn(bool cond) {
        uint16_t addr = argword();
        if (cond) {
            REG.TCLK = 16;
            REG.PC   = addr;
        } else {
            REG.TCLK = 12;
        }
    }

    void jp_atHL() {
        REG.TCLK = 4;
        REG.PC   = REG.HL;
    }

    void jr_e() {
        uint8_t n = argbyte();
        int8_t e  = *reinterpret_cast<int8_t *>(&n);
        stuck_flag |= (e == -2);
        REG.PC += e;
        REG.TCLK = 12;
    }

    void jr_f_e(bool cond) {
        uint8_t n = argbyte();
        if (cond) {
            int8_t e = *reinterpret_cast<int8_t *>(&n);
            REG.TCLK = 12;
            stuck_flag |= (e == -2);
            REG.PC += e;
        } else {
            REG.TCLK = 8;
        }
    }

    // interrupts

    void ei() {
        REG.IME  = 1;
        REG.TCLK = 4;
    }

    void di() {
        REG.IME  = 0;
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

    void call_f_nn(bool cond) {
        uint16_t nn = argword();
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
        REG.PC   = addr;
        REG.TCLK = 16;
    }

    void rsti(uint8_t addr) {
        writeWord(REG.SP - 2, REG.PC);
        REG.SP -= 2;
        REG.PC   = addr;
        REG.TCLK = 16;
    }

    // ret

    void ret() {
        REG.PC = readWord(REG.SP);
        REG.SP += 2;
        REG.TCLK = 16;
    }

    void ret_f(bool cond) {
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
        REG.IME  = 1;
    }

    void halt() {
        REG.HALT = 1;
        REG.TCLK = 16;
    }

    void daa() {

        int a = REG.A;

        if (!REG.FLAG_N) {
            if (REG.FLAG_H || (a & 0x0F) > 0x09)
                a += 0x06;
            if (REG.FLAG_C || a > 0x9F)
                a += 0x60;
        } else {
            if (REG.FLAG_H)
                a = (a - 6) & 0xFF;
            if (REG.FLAG_C)
                a -= 0x60;
        }

        REG.FLAG_H = 0;
        REG.FLAG_C = (REG.FLAG_C || (a & 0x100) == 0x100);
        a &= 0xFF;
        REG.FLAG_Z = (a == 0);

        REG.A    = (uint8_t)a;
        REG.TCLK = 4;
    }

    void scf() {
        REG.FLAG_C = 1;
        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.TCLK   = 4;
    }

    void ccf() {
        REG.FLAG_C = (!REG.FLAG_C);
        REG.FLAG_H = 0;
        REG.FLAG_N = 0;
        REG.TCLK   = 4;
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

    void ext() {
        uint8_t opcode = argbyte();
        ext_instructions[opcode].fn(*this);
    }
};