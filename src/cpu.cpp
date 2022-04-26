#include "cpu.h"
#include "mem.h"

uint8_t Cpu::readByte(uint16_t addr) {
    return MEM.readByte(addr);
}

uint16_t Cpu::readWord(uint16_t addr) {
    return MEM.readWord(addr);
}

void Cpu::writeByte(uint16_t addr, uint8_t val) {
    MEM.writeByte(addr, val);
}

void Cpu::writeWord(uint16_t addr, uint16_t val) {
    MEM.writeWord(addr, val);
}

void Cpu::handle_interrupts() {
    uint8_t trigger = *MEM.IE & *MEM.IF;
    if ((REG.IME || REG.HALT) && trigger) {
        REG.HALT = 0;
        if (!REG.IME)
            return;
        REG.IME = 0;
        if (trigger & FLAG_IF_VBLANK) {
            *MEM.IF ^= FLAG_IF_VBLANK;
            rsti(ISR_VBLANK);
        } else if (trigger & FLAG_IF_LCD) {
            *MEM.IF ^= FLAG_IF_LCD;
            rsti(ISR_LCD);
        } else if (trigger & FLAG_IF_TIMER) {
            *MEM.IF ^= FLAG_IF_TIMER;
            rsti(ISR_TIMER);
        } else if (trigger & FLAG_IF_SERIAL) {
            *MEM.IF ^= FLAG_IF_SERIAL;
            rsti(ISR_SERIAL);
        } else { // if (trigger & FLAG_IF_JOYPAD) {
            *MEM.IF ^= FLAG_IF_JOYPAD;
            rsti(ISR_JOYPAD);
        }
    }
}

void Cpu::init_instructions() {
    instructions[0x00] = Cpu::Instruction{"NOP", 0, [](Cpu &CPU) { CPU.nop(); }};
    instructions[0x01] = Cpu::Instruction{"LD BC, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.BC); }};
    instructions[0x02] = Cpu::Instruction{"LD (BC), A", 0, [](Cpu &CPU) { CPU.ld_atrw_A(&CPU.REG.BC); }};
    instructions[0x03] = Cpu::Instruction{"INC BC", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.BC); }};
    instructions[0x04] = Cpu::Instruction{"INC B", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.B); }};
    instructions[0x05] = Cpu::Instruction{"DEC B", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.B); }};
    instructions[0x06] = Cpu::Instruction{"LD B, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.B); }};
    instructions[0x07] = Cpu::Instruction{"RLCA", 0, [](Cpu &CPU) { CPU.rlca(); }};
    instructions[0x08] = Cpu::Instruction{"LD (0x%04X), SP", 2, [](Cpu &CPU) { CPU.ld_atnn_SP(); }};
    instructions[0x09] = Cpu::Instruction{"ADD HL, BC", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.BC); }};
    instructions[0x0A] = Cpu::Instruction{"LD A, (BC)", 0, [](Cpu &CPU) { CPU.ld_A_atrw(&CPU.REG.BC); }};
    instructions[0x0B] = Cpu::Instruction{"DEC BC", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.BC); }};
    instructions[0x0C] = Cpu::Instruction{"INC C", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.C); }};
    instructions[0x0D] = Cpu::Instruction{"DEC C", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.C); }};
    instructions[0x0E] = Cpu::Instruction{"LD C, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.C); }};
    instructions[0x0F] = Cpu::Instruction{"RRCA", 0, [](Cpu &CPU) { CPU.rrca(); }};

    instructions[0x10] = Cpu::Instruction{"STOP", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0x11] = Cpu::Instruction{"LD DE, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.DE); }};
    instructions[0x12] = Cpu::Instruction{"LD (DE), A", 0, [](Cpu &CPU) { CPU.ld_atrw_A(&CPU.REG.DE); }};
    instructions[0x13] = Cpu::Instruction{"INC DE", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.DE); }};
    instructions[0x14] = Cpu::Instruction{"INC D", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.D); }};
    instructions[0x15] = Cpu::Instruction{"DEC D", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.D); }};
    instructions[0x16] = Cpu::Instruction{"LD D, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.D); }};
    instructions[0x17] = Cpu::Instruction{"RLA", 0, [](Cpu &CPU) { CPU.rla(); }};
    instructions[0x18] = Cpu::Instruction{"JR 0x%02X", 1, [](Cpu &CPU) { CPU.jr_e(); }};
    instructions[0x19] = Cpu::Instruction{"ADD HL, DE", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.DE); }};
    instructions[0x1A] = Cpu::Instruction{"LD A, (DE)", 0, [](Cpu &CPU) { CPU.ld_A_atrw(&CPU.REG.DE); }};
    instructions[0x1B] = Cpu::Instruction{"DEC DE", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.DE); }};
    instructions[0x1C] = Cpu::Instruction{"INC E", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.E); }};
    instructions[0x1D] = Cpu::Instruction{"DEC E", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.E); }};
    instructions[0x1E] = Cpu::Instruction{"LD E, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.E); }};
    instructions[0x1F] = Cpu::Instruction{"RRA", 0, [](Cpu &CPU) { CPU.rra(); }};

    instructions[0x20] = Cpu::Instruction{"JR NZ, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(!CPU.REG.FLAG_Z); }};
    instructions[0x21] = Cpu::Instruction{"LD HL, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.HL); }};
    instructions[0x22] = Cpu::Instruction{"LDI (HL), A", 0, [](Cpu &CPU) { CPU.ldi_atHL_A(); }};
    instructions[0x23] = Cpu::Instruction{"INC HL", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.HL); }};
    instructions[0x24] = Cpu::Instruction{"INC H", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.H); }};
    instructions[0x25] = Cpu::Instruction{"DEC H", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.H); }};
    instructions[0x26] = Cpu::Instruction{"LD H, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.H); }};
    instructions[0x27] = Cpu::Instruction{"DAA", 0, [](Cpu &CPU) { CPU.daa(); }};
    instructions[0x28] = Cpu::Instruction{"JR Z, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(CPU.REG.FLAG_Z); }};
    instructions[0x29] = Cpu::Instruction{"ADD HL, HL", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.HL); }};
    instructions[0x2A] = Cpu::Instruction{"LDI A, (HL)", 0, [](Cpu &CPU) { CPU.ldi_A_atHL(); }};
    instructions[0x2B] = Cpu::Instruction{"DEC HL", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.HL); }};
    instructions[0x2C] = Cpu::Instruction{"INC L", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.L); }};
    instructions[0x2D] = Cpu::Instruction{"DEC L", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.L); }};
    instructions[0x2E] = Cpu::Instruction{"LD L, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.L); }};
    instructions[0x2F] = Cpu::Instruction{"CPL", 0, [](Cpu &CPU) { CPU.cpl(); }};

    instructions[0x30] = Cpu::Instruction{"JR NC, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(!CPU.REG.FLAG_C); }};
    instructions[0x31] = Cpu::Instruction{"LD SP, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.SP); }};
    instructions[0x32] = Cpu::Instruction{"LDD (HL), A", 0, [](Cpu &CPU) { CPU.ldd_atHL_A(); }};
    instructions[0x33] = Cpu::Instruction{"INC SP", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.SP); }};
    instructions[0x34] = Cpu::Instruction{"INC (HL)", 0, [](Cpu &CPU) { CPU.inc_atHL(); }};
    instructions[0x35] = Cpu::Instruction{"DEC (HL)", 0, [](Cpu &CPU) { CPU.dec_atHL(); }};
    instructions[0x36] = Cpu::Instruction{"LD (HL), 0x%02X", 1, [](Cpu &CPU) { CPU.ld_atHL_n(); }};
    instructions[0x37] = Cpu::Instruction{"SCF", 0, [](Cpu &CPU) { CPU.scf(); }};
    instructions[0x38] = Cpu::Instruction{"JR C, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(CPU.REG.FLAG_C); }};
    instructions[0x39] = Cpu::Instruction{"ADD HL, SP", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.SP); }};
    instructions[0x3A] = Cpu::Instruction{"LDD A, (HL)", 0, [](Cpu &CPU) { CPU.ldd_A_atHL(); }};
    instructions[0x3B] = Cpu::Instruction{"DEC SP", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.SP); }};
    instructions[0x3C] = Cpu::Instruction{"INC A", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.A); }};
    instructions[0x3D] = Cpu::Instruction{"DEC A", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.A); }};
    instructions[0x3E] = Cpu::Instruction{"LD A, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.A); }};
    instructions[0x3F] = Cpu::Instruction{"CCF", 0, [](Cpu &CPU) { CPU.ccf(); }};

    instructions[0x40] = Cpu::Instruction{"LD B, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.B); }};
    instructions[0x41] = Cpu::Instruction{"LD B, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.C); }};
    instructions[0x42] = Cpu::Instruction{"LD B, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.D); }};
    instructions[0x43] = Cpu::Instruction{"LD B, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.E); }};
    instructions[0x44] = Cpu::Instruction{"LD B, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.H); }};
    instructions[0x45] = Cpu::Instruction{"LD B, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.L); }};
    instructions[0x46] = Cpu::Instruction{"LD B, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.B); }};
    instructions[0x47] = Cpu::Instruction{"LD B, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.A); }};
    instructions[0x48] = Cpu::Instruction{"LD C, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.B); }};
    instructions[0x49] = Cpu::Instruction{"LD C, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.C); }};
    instructions[0x4A] = Cpu::Instruction{"LD C, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.D); }};
    instructions[0x4B] = Cpu::Instruction{"LD C, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.E); }};
    instructions[0x4C] = Cpu::Instruction{"LD C, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.H); }};
    instructions[0x4D] = Cpu::Instruction{"LD C, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.L); }};
    instructions[0x4E] = Cpu::Instruction{"LD C, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.C); }};
    instructions[0x4F] = Cpu::Instruction{"LD C, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.A); }};

    instructions[0x50] = Cpu::Instruction{"LD D, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.B); }};
    instructions[0x51] = Cpu::Instruction{"LD D, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.C); }};
    instructions[0x52] = Cpu::Instruction{"LD D, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.D); }};
    instructions[0x53] = Cpu::Instruction{"LD D, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.E); }};
    instructions[0x54] = Cpu::Instruction{"LD D, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.H); }};
    instructions[0x55] = Cpu::Instruction{"LD D, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.L); }};
    instructions[0x56] = Cpu::Instruction{"LD D, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.D); }};
    instructions[0x57] = Cpu::Instruction{"LD D, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.A); }};
    instructions[0x58] = Cpu::Instruction{"LD E, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.B); }};
    instructions[0x59] = Cpu::Instruction{"LD E, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.C); }};
    instructions[0x5A] = Cpu::Instruction{"LD E, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.D); }};
    instructions[0x5B] = Cpu::Instruction{"LD E, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.E); }};
    instructions[0x5C] = Cpu::Instruction{"LD E, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.H); }};
    instructions[0x5D] = Cpu::Instruction{"LD E, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.L); }};
    instructions[0x5E] = Cpu::Instruction{"LD E, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.E); }};
    instructions[0x5F] = Cpu::Instruction{"LD E, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.A); }};

    instructions[0x60] = Cpu::Instruction{"LD H, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.B); }};
    instructions[0x61] = Cpu::Instruction{"LD H, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.C); }};
    instructions[0x62] = Cpu::Instruction{"LD H, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.D); }};
    instructions[0x63] = Cpu::Instruction{"LD H, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.E); }};
    instructions[0x64] = Cpu::Instruction{"LD H, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.H); }};
    instructions[0x65] = Cpu::Instruction{"LD H, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.L); }};
    instructions[0x66] = Cpu::Instruction{"LD H, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.H); }};
    instructions[0x67] = Cpu::Instruction{"LD H, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.A); }};
    instructions[0x68] = Cpu::Instruction{"LD L, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.B); }};
    instructions[0x69] = Cpu::Instruction{"LD L, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.C); }};
    instructions[0x6A] = Cpu::Instruction{"LD L, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.D); }};
    instructions[0x6B] = Cpu::Instruction{"LD L, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.E); }};
    instructions[0x6C] = Cpu::Instruction{"LD L, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.H); }};
    instructions[0x6D] = Cpu::Instruction{"LD L, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.L); }};
    instructions[0x6E] = Cpu::Instruction{"LD L, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.L); }};
    instructions[0x6F] = Cpu::Instruction{"LD L, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.A); }};

    instructions[0x70] = Cpu::Instruction{"LD (HL), B", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.B); }};
    instructions[0x71] = Cpu::Instruction{"LD (HL), C", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.C); }};
    instructions[0x72] = Cpu::Instruction{"LD (HL), D", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.D); }};
    instructions[0x73] = Cpu::Instruction{"LD (HL), E", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.E); }};
    instructions[0x74] = Cpu::Instruction{"LD (HL), H", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.H); }};
    instructions[0x75] = Cpu::Instruction{"LD (HL), L", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.L); }};
    instructions[0x76] = Cpu::Instruction{"HALT", 0, [](Cpu &CPU) { CPU.halt(); }};
    instructions[0x77] = Cpu::Instruction{"LD (HL), A", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.A); }};
    instructions[0x78] = Cpu::Instruction{"LD A, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.B); }};
    instructions[0x79] = Cpu::Instruction{"LD A, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.C); }};
    instructions[0x7A] = Cpu::Instruction{"LD A, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.D); }};
    instructions[0x7B] = Cpu::Instruction{"LD A, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.E); }};
    instructions[0x7C] = Cpu::Instruction{"LD A, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.H); }};
    instructions[0x7D] = Cpu::Instruction{"LD A, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.L); }};
    instructions[0x7E] = Cpu::Instruction{"LD A, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.A); }};
    instructions[0x7F] = Cpu::Instruction{"LD A, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.A); }};

    instructions[0x80] = Cpu::Instruction{"ADD A, B", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.B); }};
    instructions[0x81] = Cpu::Instruction{"ADD A, C", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.C); }};
    instructions[0x82] = Cpu::Instruction{"ADD A, D", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.D); }};
    instructions[0x83] = Cpu::Instruction{"ADD A, E", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.E); }};
    instructions[0x84] = Cpu::Instruction{"ADD A, H", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.H); }};
    instructions[0x85] = Cpu::Instruction{"ADD A, L", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.L); }};
    instructions[0x86] = Cpu::Instruction{"ADD A, (HL)", 0, [](Cpu &CPU) { CPU.add_A_atHL(); }};
    instructions[0x87] = Cpu::Instruction{"ADD A, A", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.A); }};
    instructions[0x88] = Cpu::Instruction{"ADC A, B", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.B); }};
    instructions[0x89] = Cpu::Instruction{"ADC A, C", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.C); }};
    instructions[0x8A] = Cpu::Instruction{"ADC A, D", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.D); }};
    instructions[0x8B] = Cpu::Instruction{"ADC A, E", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.E); }};
    instructions[0x8C] = Cpu::Instruction{"ADC A, H", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.H); }};
    instructions[0x8D] = Cpu::Instruction{"ADC A, L", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.L); }};
    instructions[0x8E] = Cpu::Instruction{"ADC A, (HL)", 0, [](Cpu &CPU) { CPU.adc_A_atHL(); }};
    instructions[0x8F] = Cpu::Instruction{"ADC A, A", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.A); }};

    instructions[0x90] = Cpu::Instruction{"SUB A, B", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.B); }};
    instructions[0x91] = Cpu::Instruction{"SUB A, C", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.C); }};
    instructions[0x92] = Cpu::Instruction{"SUB A, D", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.D); }};
    instructions[0x93] = Cpu::Instruction{"SUB A, E", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.E); }};
    instructions[0x94] = Cpu::Instruction{"SUB A, H", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.H); }};
    instructions[0x95] = Cpu::Instruction{"SUB A, L", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.L); }};
    instructions[0x96] = Cpu::Instruction{"SUB A, (HL)", 0, [](Cpu &CPU) { CPU.sub_A_atHL(); }};
    instructions[0x97] = Cpu::Instruction{"SUB A, A", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.A); }};
    instructions[0x98] = Cpu::Instruction{"SBC A, B", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.B); }};
    instructions[0x99] = Cpu::Instruction{"SBC A, C", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.C); }};
    instructions[0x9A] = Cpu::Instruction{"SBC A, D", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.D); }};
    instructions[0x9B] = Cpu::Instruction{"SBC A, E", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.E); }};
    instructions[0x9C] = Cpu::Instruction{"SBC A, H", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.H); }};
    instructions[0x9D] = Cpu::Instruction{"SBC A, L", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.L); }};
    instructions[0x9E] = Cpu::Instruction{"SBC A, (HL)", 0, [](Cpu &CPU) { CPU.sbc_A_atHL(); }};
    instructions[0x9F] = Cpu::Instruction{"SBC A, A", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.A); }};

    instructions[0xA0] = Cpu::Instruction{"AND B", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.B); }};
    instructions[0xA1] = Cpu::Instruction{"AND C", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.C); }};
    instructions[0xA2] = Cpu::Instruction{"AND D", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.D); }};
    instructions[0xA3] = Cpu::Instruction{"AND E", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.E); }};
    instructions[0xA4] = Cpu::Instruction{"AND H", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.H); }};
    instructions[0xA5] = Cpu::Instruction{"AND L", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.L); }};
    instructions[0xA6] = Cpu::Instruction{"AND (HL)", 0, [](Cpu &CPU) { CPU.and_atHL(); }};
    instructions[0xA7] = Cpu::Instruction{"AND A", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.A); }};
    instructions[0xA8] = Cpu::Instruction{"XOR B", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.B); }};
    instructions[0xA9] = Cpu::Instruction{"XOR C", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.C); }};
    instructions[0xAA] = Cpu::Instruction{"XOR D", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.D); }};
    instructions[0xAB] = Cpu::Instruction{"XOR E", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.E); }};
    instructions[0xAC] = Cpu::Instruction{"XOR H", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.H); }};
    instructions[0xAD] = Cpu::Instruction{"XOR L", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.L); }};
    instructions[0xAE] = Cpu::Instruction{"XOR (HL)", 0, [](Cpu &CPU) { CPU.xor_atHL(); }};
    instructions[0xAF] = Cpu::Instruction{"XOR A", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.A); }};

    instructions[0xB0] = Cpu::Instruction{"OR B", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.B); }};
    instructions[0xB1] = Cpu::Instruction{"OR C", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.C); }};
    instructions[0xB2] = Cpu::Instruction{"OR D", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.D); }};
    instructions[0xB3] = Cpu::Instruction{"OR E", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.E); }};
    instructions[0xB4] = Cpu::Instruction{"OR H", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.H); }};
    instructions[0xB5] = Cpu::Instruction{"OR L", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.L); }};
    instructions[0xB6] = Cpu::Instruction{"OR (HL)", 0, [](Cpu &CPU) { CPU.or_atHL(); }};
    instructions[0xB7] = Cpu::Instruction{"OR A", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.A); }};
    instructions[0xB8] = Cpu::Instruction{"CP B", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.B); }};
    instructions[0xB9] = Cpu::Instruction{"CP C", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.C); }};
    instructions[0xBA] = Cpu::Instruction{"CP D", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.D); }};
    instructions[0xBB] = Cpu::Instruction{"CP E", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.E); }};
    instructions[0xBC] = Cpu::Instruction{"CP H", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.H); }};
    instructions[0xBD] = Cpu::Instruction{"CP L", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.L); }};
    instructions[0xBE] = Cpu::Instruction{"CP (HL)", 0, [](Cpu &CPU) { CPU.cp_atHL(); }};
    instructions[0xBF] = Cpu::Instruction{"CP A", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.A); }};

    instructions[0xC0] = Cpu::Instruction{"RET NZ", 0, [](Cpu &CPU) { CPU.ret_f(!CPU.REG.FLAG_Z); }};
    instructions[0xC1] = Cpu::Instruction{"POP BC", 0, [](Cpu &CPU) { CPU.pop_rw(&CPU.REG.BC); }};
    instructions[0xC2] = Cpu::Instruction{"JP NZ, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(!CPU.REG.FLAG_Z); }};
    instructions[0xC3] = Cpu::Instruction{"JP 0x%04X", 2, [](Cpu &CPU) { CPU.jp_nn(); }};
    instructions[0xC4] = Cpu::Instruction{"CALL NZ, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(!CPU.REG.FLAG_Z); }};
    instructions[0xC5] = Cpu::Instruction{"PUSH BC", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.BC); }};
    instructions[0xC6] = Cpu::Instruction{"ADD A, 0x%02X", 1, [](Cpu &CPU) { CPU.add_A_n(); }};
    instructions[0xC7] = Cpu::Instruction{"RST 0", 0, [](Cpu &CPU) { CPU.rst(0x00); }};
    instructions[0xC8] = Cpu::Instruction{"RET Z", 0, [](Cpu &CPU) { CPU.ret_f(CPU.REG.FLAG_Z); }};
    instructions[0xC9] = Cpu::Instruction{"RET", 0, [](Cpu &CPU) { CPU.ret(); }};
    instructions[0xCA] = Cpu::Instruction{"JP Z, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(CPU.REG.FLAG_Z); }};
    instructions[0xCB] = Cpu::Instruction{"Ext Op", 1, [](Cpu &CPU) { CPU.ext(); }};
    instructions[0xCC] = Cpu::Instruction{"CALL Z, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(CPU.REG.FLAG_Z); }};
    instructions[0xCD] = Cpu::Instruction{"CALL 0x%04X", 2, [](Cpu &CPU) { CPU.call_nn(); }};
    instructions[0xCE] = Cpu::Instruction{"ADC A, 0x%02X", 1, [](Cpu &CPU) { CPU.adc_A_n(); }};
    instructions[0xCF] = Cpu::Instruction{"RST 8", 0, [](Cpu &CPU) { CPU.rst(0x08); }};

    instructions[0xD0] = Cpu::Instruction{"RET NC", 0, [](Cpu &CPU) { CPU.ret_f(!CPU.REG.FLAG_C); }};
    instructions[0xD1] = Cpu::Instruction{"POP DE", 0, [](Cpu &CPU) { CPU.pop_rw(&CPU.REG.DE); }};
    instructions[0xD2] = Cpu::Instruction{"JP NC, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(!CPU.REG.FLAG_C); }};
    instructions[0xD3] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xD4] = Cpu::Instruction{"CALL NC, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(!CPU.REG.FLAG_C); }};
    instructions[0xD5] = Cpu::Instruction{"PUSH DE", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.DE); }};
    instructions[0xD6] = Cpu::Instruction{"SUB A, 0x%02X", 1, [](Cpu &CPU) { CPU.sub_A_n(); }};
    instructions[0xD7] = Cpu::Instruction{"RST 10", 0, [](Cpu &CPU) { CPU.rst(0x10); }};
    instructions[0xD8] = Cpu::Instruction{"RET C", 0, [](Cpu &CPU) { CPU.ret_f(CPU.REG.FLAG_C); }};
    instructions[0xD9] = Cpu::Instruction{"RETI", 0, [](Cpu &CPU) { CPU.reti(); }};
    instructions[0xDA] = Cpu::Instruction{"JP C, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(CPU.REG.FLAG_C); }};
    instructions[0xDB] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xDC] = Cpu::Instruction{"CALL C, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(CPU.REG.FLAG_C); }};
    instructions[0xDD] = Cpu::Instruction{"MEM_SENTINEL", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xDE] = Cpu::Instruction{"SBC A, 0x%02X", 1, [](Cpu &CPU) { CPU.sbc_A_n(); }};
    instructions[0xDF] = Cpu::Instruction{"RST 18", 0, [](Cpu &CPU) { CPU.rst(0x18); }};

    instructions[0xE0] = Cpu::Instruction{"LDH (0x%02X), A", 1, [](Cpu &CPU) { CPU.ldh_atn_A(); }};
    instructions[0xE1] = Cpu::Instruction{"POP HL", 0, [](Cpu &CPU) { CPU.pop_rw(&CPU.REG.HL); }};
    instructions[0xE2] = Cpu::Instruction{"LDH (C), A", 0, [](Cpu &CPU) { CPU.ldh_atC_A(); }};
    instructions[0xE3] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xE4] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xE5] = Cpu::Instruction{"PUSH HL", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.HL); }};
    instructions[0xE6] = Cpu::Instruction{"AND 0x%02X", 1, [](Cpu &CPU) { CPU.and_n(); }};
    instructions[0xE7] = Cpu::Instruction{"RST 20", 0, [](Cpu &CPU) { CPU.rst(0x20); }};
    instructions[0xE8] = Cpu::Instruction{"ADD SP, 0x%02X", 0, [](Cpu &CPU) { CPU.add_SP_e(); }};
    instructions[0xE9] = Cpu::Instruction{"JP (HL)", 0, [](Cpu &CPU) { CPU.jp_atHL(); }};
    instructions[0xEA] = Cpu::Instruction{"LD (0x%04X), A", 2, [](Cpu &CPU) { CPU.ld_atnn_A(); }};
    instructions[0xEB] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xEC] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xED] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xEE] = Cpu::Instruction{"XOR 0x%02X", 1, [](Cpu &CPU) { CPU.xor_n(); }};
    instructions[0xEF] = Cpu::Instruction{"RST 28", 0, [](Cpu &CPU) { CPU.rst(0x28); }};

    instructions[0xF0] = Cpu::Instruction{"LDH A, (0x%02X)", 1, [](Cpu &CPU) { CPU.ldh_A_atn(); }};
    instructions[0xF1] = Cpu::Instruction{"POP AF", 0, [](Cpu &CPU) { CPU.pop_AF(); }};
    instructions[0xF2] = Cpu::Instruction{"LDH A, (C)", 0, [](Cpu &CPU) { CPU.ldh_A_atC(); }};
    instructions[0xF3] = Cpu::Instruction{"DI", 0, [](Cpu &CPU) { CPU.di(); }};
    instructions[0xF4] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xF5] = Cpu::Instruction{"PUSH AF", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.AF); }};
    instructions[0xF6] = Cpu::Instruction{"OR 0x%02X", 1, [](Cpu &CPU) { CPU.or_n(); }};
    instructions[0xF7] = Cpu::Instruction{"RST 30", 0, [](Cpu &CPU) { CPU.rst(0x30); }};
    instructions[0xF8] = Cpu::Instruction{"LDHL SP, 0x%02X", 0, [](Cpu &CPU) { CPU.ld_HL_SP_e(); }};
    instructions[0xF9] = Cpu::Instruction{"LD SP, HL", 0, [](Cpu &CPU) { CPU.ld_SP_HL(); }};
    instructions[0xFA] = Cpu::Instruction{"LD A, (0x%04X)", 2, [](Cpu &CPU) { CPU.ld_A_atnn(); }};
    instructions[0xFB] = Cpu::Instruction{"EI", 0, [](Cpu &CPU) { CPU.ei(); }};
    instructions[0xFC] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xFD] = Cpu::Instruction{"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xFE] = Cpu::Instruction{"CP 0x%02X", 1, [](Cpu &CPU) { CPU.cp_n(); }};
    instructions[0xFF] = Cpu::Instruction{"RST 38", 0, [](Cpu &CPU) { CPU.rst(0x38); }};
}

void Cpu::init_ext_instructions() {
    ext_instructions[0x00] = Cpu::Instruction{"RLC B", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.B); }};
    ext_instructions[0x01] = Cpu::Instruction{"RLC C", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.C); }};
    ext_instructions[0x02] = Cpu::Instruction{"RLC D", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.D); }};
    ext_instructions[0x03] = Cpu::Instruction{"RLC E", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.E); }};
    ext_instructions[0x04] = Cpu::Instruction{"RLC H", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.H); }};
    ext_instructions[0x05] = Cpu::Instruction{"RLC L", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.L); }};
    ext_instructions[0x06] = Cpu::Instruction{"RLC (HL)", 0, [](Cpu &CPU) { CPU.rlc_atHL(); }};
    ext_instructions[0x07] = Cpu::Instruction{"RLC A", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.A); }};
    ext_instructions[0x08] = Cpu::Instruction{"RRC B", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.B); }};
    ext_instructions[0x09] = Cpu::Instruction{"RRC C", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.C); }};
    ext_instructions[0x0A] = Cpu::Instruction{"RRC D", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.D); }};
    ext_instructions[0x0B] = Cpu::Instruction{"RRC E", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.E); }};
    ext_instructions[0x0C] = Cpu::Instruction{"RRC H", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.H); }};
    ext_instructions[0x0D] = Cpu::Instruction{"RRC L", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.L); }};
    ext_instructions[0x0E] = Cpu::Instruction{"RRC (HL)", 0, [](Cpu &CPU) { CPU.rrc_atHL(); }};
    ext_instructions[0x0F] = Cpu::Instruction{"RRC A", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.A); }};

    ext_instructions[0x10] = Cpu::Instruction{"RL B", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.B); }};
    ext_instructions[0x11] = Cpu::Instruction{"RL C", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.C); }};
    ext_instructions[0x12] = Cpu::Instruction{"RL D", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.D); }};
    ext_instructions[0x13] = Cpu::Instruction{"RL E", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.E); }};
    ext_instructions[0x14] = Cpu::Instruction{"RL H", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.H); }};
    ext_instructions[0x15] = Cpu::Instruction{"RL L", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.L); }};
    ext_instructions[0x16] = Cpu::Instruction{"RL (HL)", 0, [](Cpu &CPU) { CPU.rl_atHL(); }};
    ext_instructions[0x17] = Cpu::Instruction{"RL A", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.A); }};
    ext_instructions[0x18] = Cpu::Instruction{"RR B", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.B); }};
    ext_instructions[0x19] = Cpu::Instruction{"RR C", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.C); }};
    ext_instructions[0x1A] = Cpu::Instruction{"RR D", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.D); }};
    ext_instructions[0x1B] = Cpu::Instruction{"RR E", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.E); }};
    ext_instructions[0x1C] = Cpu::Instruction{"RR H", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.H); }};
    ext_instructions[0x1D] = Cpu::Instruction{"RR L", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.L); }};
    ext_instructions[0x1E] = Cpu::Instruction{"RR (HL)", 0, [](Cpu &CPU) { CPU.rr_atHL(); }};
    ext_instructions[0x1F] = Cpu::Instruction{"RR A", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.A); }};

    ext_instructions[0x20] = Cpu::Instruction{"SLA B", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.B); }};
    ext_instructions[0x21] = Cpu::Instruction{"SLA C", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.C); }};
    ext_instructions[0x22] = Cpu::Instruction{"SLA D", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.D); }};
    ext_instructions[0x23] = Cpu::Instruction{"SLA E", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.E); }};
    ext_instructions[0x24] = Cpu::Instruction{"SLA H", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.H); }};
    ext_instructions[0x25] = Cpu::Instruction{"SLA L", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.L); }};
    ext_instructions[0x26] = Cpu::Instruction{"SLA (HL)", 0, [](Cpu &CPU) { CPU.sla_atHL(); }};
    ext_instructions[0x27] = Cpu::Instruction{"SLA A", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.A); }};
    ext_instructions[0x28] = Cpu::Instruction{"SRA B", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.B); }};
    ext_instructions[0x29] = Cpu::Instruction{"SRA C", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.C); }};
    ext_instructions[0x2A] = Cpu::Instruction{"SRA D", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.D); }};
    ext_instructions[0x2B] = Cpu::Instruction{"SRA E", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.E); }};
    ext_instructions[0x2C] = Cpu::Instruction{"SRA H", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.H); }};
    ext_instructions[0x2D] = Cpu::Instruction{"SRA L", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.L); }};
    ext_instructions[0x2E] = Cpu::Instruction{"SRA (HL)", 0, [](Cpu &CPU) { CPU.sra_atHL(); }};
    ext_instructions[0x2F] = Cpu::Instruction{"SRA A", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.A); }};

    ext_instructions[0x30] = Cpu::Instruction{"SWAP B", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.B); }};
    ext_instructions[0x31] = Cpu::Instruction{"SWAP C", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.C); }};
    ext_instructions[0x32] = Cpu::Instruction{"SWAP D", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.D); }};
    ext_instructions[0x33] = Cpu::Instruction{"SWAP E", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.E); }};
    ext_instructions[0x34] = Cpu::Instruction{"SWAP H", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.H); }};
    ext_instructions[0x35] = Cpu::Instruction{"SWAP L", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.L); }};
    ext_instructions[0x36] = Cpu::Instruction{"SWAP (HL)", 0, [](Cpu &CPU) { CPU.swap_atHL(); }};
    ext_instructions[0x37] = Cpu::Instruction{"SWAP A", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.A); }};
    ext_instructions[0x38] = Cpu::Instruction{"SRL B", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.B); }};
    ext_instructions[0x39] = Cpu::Instruction{"SRL C", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.C); }};
    ext_instructions[0x3A] = Cpu::Instruction{"SRL D", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.D); }};
    ext_instructions[0x3B] = Cpu::Instruction{"SRL E", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.E); }};
    ext_instructions[0x3C] = Cpu::Instruction{"SRL H", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.H); }};
    ext_instructions[0x3D] = Cpu::Instruction{"SRL L", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.L); }};
    ext_instructions[0x3E] = Cpu::Instruction{"SRL (HL)", 0, [](Cpu &CPU) { CPU.srl_atHL(); }};
    ext_instructions[0x3F] = Cpu::Instruction{"SRL A", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.A); }};

    ext_instructions[0x40] = Cpu::Instruction{"BIT 0, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.B); }};
    ext_instructions[0x41] = Cpu::Instruction{"BIT 0, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.C); }};
    ext_instructions[0x42] = Cpu::Instruction{"BIT 0, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.D); }};
    ext_instructions[0x43] = Cpu::Instruction{"BIT 0, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.E); }};
    ext_instructions[0x44] = Cpu::Instruction{"BIT 0, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.H); }};
    ext_instructions[0x45] = Cpu::Instruction{"BIT 0, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.L); }};
    ext_instructions[0x46] = Cpu::Instruction{"BIT 0, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_0); }};
    ext_instructions[0x47] = Cpu::Instruction{"BIT 0, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.A); }};
    ext_instructions[0x48] = Cpu::Instruction{"BIT 1, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.B); }};
    ext_instructions[0x49] = Cpu::Instruction{"BIT 1, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.C); }};
    ext_instructions[0x4A] = Cpu::Instruction{"BIT 1, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.D); }};
    ext_instructions[0x4B] = Cpu::Instruction{"BIT 1, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.E); }};
    ext_instructions[0x4C] = Cpu::Instruction{"BIT 1, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.H); }};
    ext_instructions[0x4D] = Cpu::Instruction{"BIT 1, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.L); }};
    ext_instructions[0x4E] = Cpu::Instruction{"BIT 1, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_1); }};
    ext_instructions[0x4F] = Cpu::Instruction{"BIT 1, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.A); }};

    ext_instructions[0x50] = Cpu::Instruction{"BIT 2, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.B); }};
    ext_instructions[0x51] = Cpu::Instruction{"BIT 2, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.C); }};
    ext_instructions[0x52] = Cpu::Instruction{"BIT 2, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.D); }};
    ext_instructions[0x53] = Cpu::Instruction{"BIT 2, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.E); }};
    ext_instructions[0x54] = Cpu::Instruction{"BIT 2, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.H); }};
    ext_instructions[0x55] = Cpu::Instruction{"BIT 2, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.L); }};
    ext_instructions[0x56] = Cpu::Instruction{"BIT 2, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_2); }};
    ext_instructions[0x57] = Cpu::Instruction{"BIT 2, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.A); }};
    ext_instructions[0x58] = Cpu::Instruction{"BIT 3, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.B); }};
    ext_instructions[0x59] = Cpu::Instruction{"BIT 3, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.C); }};
    ext_instructions[0x5A] = Cpu::Instruction{"BIT 3, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.D); }};
    ext_instructions[0x5B] = Cpu::Instruction{"BIT 3, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.E); }};
    ext_instructions[0x5C] = Cpu::Instruction{"BIT 3, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.H); }};
    ext_instructions[0x5D] = Cpu::Instruction{"BIT 3, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.L); }};
    ext_instructions[0x5E] = Cpu::Instruction{"BIT 3, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_3); }};
    ext_instructions[0x5F] = Cpu::Instruction{"BIT 3, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.A); }};

    ext_instructions[0x60] = Cpu::Instruction{"BIT 4, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.B); }};
    ext_instructions[0x61] = Cpu::Instruction{"BIT 4, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.C); }};
    ext_instructions[0x62] = Cpu::Instruction{"BIT 4, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.D); }};
    ext_instructions[0x63] = Cpu::Instruction{"BIT 4, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.E); }};
    ext_instructions[0x64] = Cpu::Instruction{"BIT 4, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.H); }};
    ext_instructions[0x65] = Cpu::Instruction{"BIT 4, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.L); }};
    ext_instructions[0x66] = Cpu::Instruction{"BIT 4, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_4); }};
    ext_instructions[0x67] = Cpu::Instruction{"BIT 4, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.A); }};
    ext_instructions[0x68] = Cpu::Instruction{"BIT 5, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.B); }};
    ext_instructions[0x69] = Cpu::Instruction{"BIT 5, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.C); }};
    ext_instructions[0x6A] = Cpu::Instruction{"BIT 5, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.D); }};
    ext_instructions[0x6B] = Cpu::Instruction{"BIT 5, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.E); }};
    ext_instructions[0x6C] = Cpu::Instruction{"BIT 5, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.H); }};
    ext_instructions[0x6D] = Cpu::Instruction{"BIT 5, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.L); }};
    ext_instructions[0x6E] = Cpu::Instruction{"BIT 5, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_5); }};
    ext_instructions[0x6F] = Cpu::Instruction{"BIT 5, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.A); }};

    ext_instructions[0x70] = Cpu::Instruction{"BIT 6, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.B); }};
    ext_instructions[0x71] = Cpu::Instruction{"BIT 6, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.C); }};
    ext_instructions[0x72] = Cpu::Instruction{"BIT 6, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.D); }};
    ext_instructions[0x73] = Cpu::Instruction{"BIT 6, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.E); }};
    ext_instructions[0x74] = Cpu::Instruction{"BIT 6, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.H); }};
    ext_instructions[0x75] = Cpu::Instruction{"BIT 6, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.L); }};
    ext_instructions[0x76] = Cpu::Instruction{"BIT 6, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_6); }};
    ext_instructions[0x77] = Cpu::Instruction{"BIT 6, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.A); }};
    ext_instructions[0x78] = Cpu::Instruction{"BIT 7, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.B); }};
    ext_instructions[0x79] = Cpu::Instruction{"BIT 7, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.C); }};
    ext_instructions[0x7A] = Cpu::Instruction{"BIT 7, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.D); }};
    ext_instructions[0x7B] = Cpu::Instruction{"BIT 7, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.E); }};
    ext_instructions[0x7C] = Cpu::Instruction{"BIT 7, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.H); }};
    ext_instructions[0x7D] = Cpu::Instruction{"BIT 7, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.L); }};
    ext_instructions[0x7E] = Cpu::Instruction{"BIT 7, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_7); }};
    ext_instructions[0x7F] = Cpu::Instruction{"BIT 7, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.A); }};

    ext_instructions[0x80] = Cpu::Instruction{"RES 0, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.B); }};
    ext_instructions[0x81] = Cpu::Instruction{"RES 0, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.C); }};
    ext_instructions[0x82] = Cpu::Instruction{"RES 0, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.D); }};
    ext_instructions[0x83] = Cpu::Instruction{"RES 0, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.E); }};
    ext_instructions[0x84] = Cpu::Instruction{"RES 0, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.H); }};
    ext_instructions[0x85] = Cpu::Instruction{"RES 0, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.L); }};
    ext_instructions[0x86] = Cpu::Instruction{"RES 0, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_0); }};
    ext_instructions[0x87] = Cpu::Instruction{"RES 0, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.A); }};
    ext_instructions[0x88] = Cpu::Instruction{"RES 1, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.B); }};
    ext_instructions[0x89] = Cpu::Instruction{"RES 1, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.C); }};
    ext_instructions[0x8A] = Cpu::Instruction{"RES 1, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.D); }};
    ext_instructions[0x8B] = Cpu::Instruction{"RES 1, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.E); }};
    ext_instructions[0x8C] = Cpu::Instruction{"RES 1, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.H); }};
    ext_instructions[0x8D] = Cpu::Instruction{"RES 1, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.L); }};
    ext_instructions[0x8E] = Cpu::Instruction{"RES 1, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_1); }};
    ext_instructions[0x8F] = Cpu::Instruction{"RES 1, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.A); }};

    ext_instructions[0x90] = Cpu::Instruction{"RES 2, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.B); }};
    ext_instructions[0x91] = Cpu::Instruction{"RES 2, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.C); }};
    ext_instructions[0x92] = Cpu::Instruction{"RES 2, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.D); }};
    ext_instructions[0x93] = Cpu::Instruction{"RES 2, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.E); }};
    ext_instructions[0x94] = Cpu::Instruction{"RES 2, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.H); }};
    ext_instructions[0x95] = Cpu::Instruction{"RES 2, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.L); }};
    ext_instructions[0x96] = Cpu::Instruction{"RES 2, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_2); }};
    ext_instructions[0x97] = Cpu::Instruction{"RES 2, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.A); }};
    ext_instructions[0x98] = Cpu::Instruction{"RES 3, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.B); }};
    ext_instructions[0x99] = Cpu::Instruction{"RES 3, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.C); }};
    ext_instructions[0x9A] = Cpu::Instruction{"RES 3, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.D); }};
    ext_instructions[0x9B] = Cpu::Instruction{"RES 3, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.E); }};
    ext_instructions[0x9C] = Cpu::Instruction{"RES 3, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.H); }};
    ext_instructions[0x9D] = Cpu::Instruction{"RES 3, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.L); }};
    ext_instructions[0x9E] = Cpu::Instruction{"RES 3, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_3); }};
    ext_instructions[0x9F] = Cpu::Instruction{"RES 3, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.A); }};

    ext_instructions[0xA0] = Cpu::Instruction{"RES 4, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.B); }};
    ext_instructions[0xA1] = Cpu::Instruction{"RES 4, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.C); }};
    ext_instructions[0xA2] = Cpu::Instruction{"RES 4, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.D); }};
    ext_instructions[0xA3] = Cpu::Instruction{"RES 4, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.E); }};
    ext_instructions[0xA4] = Cpu::Instruction{"RES 4, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.H); }};
    ext_instructions[0xA5] = Cpu::Instruction{"RES 4, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.L); }};
    ext_instructions[0xA6] = Cpu::Instruction{"RES 4, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_4); }};
    ext_instructions[0xA7] = Cpu::Instruction{"RES 4, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.A); }};
    ext_instructions[0xA8] = Cpu::Instruction{"RES 5, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.B); }};
    ext_instructions[0xA9] = Cpu::Instruction{"RES 5, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.C); }};
    ext_instructions[0xAA] = Cpu::Instruction{"RES 5, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.D); }};
    ext_instructions[0xAB] = Cpu::Instruction{"RES 5, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.E); }};
    ext_instructions[0xAC] = Cpu::Instruction{"RES 5, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.H); }};
    ext_instructions[0xAD] = Cpu::Instruction{"RES 5, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.L); }};
    ext_instructions[0xAE] = Cpu::Instruction{"RES 5, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_5); }};
    ext_instructions[0xAF] = Cpu::Instruction{"RES 5, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.A); }};

    ext_instructions[0xB0] = Cpu::Instruction{"RES 6, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.B); }};
    ext_instructions[0xB1] = Cpu::Instruction{"RES 6, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.C); }};
    ext_instructions[0xB2] = Cpu::Instruction{"RES 6, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.D); }};
    ext_instructions[0xB3] = Cpu::Instruction{"RES 6, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.E); }};
    ext_instructions[0xB4] = Cpu::Instruction{"RES 6, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.H); }};
    ext_instructions[0xB5] = Cpu::Instruction{"RES 6, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.L); }};
    ext_instructions[0xB6] = Cpu::Instruction{"RES 6, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_6); }};
    ext_instructions[0xB7] = Cpu::Instruction{"RES 6, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.A); }};
    ext_instructions[0xB8] = Cpu::Instruction{"RES 7, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.B); }};
    ext_instructions[0xB9] = Cpu::Instruction{"RES 7, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.C); }};
    ext_instructions[0xBA] = Cpu::Instruction{"RES 7, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.D); }};
    ext_instructions[0xBB] = Cpu::Instruction{"RES 7, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.E); }};
    ext_instructions[0xBC] = Cpu::Instruction{"RES 7, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.H); }};
    ext_instructions[0xBD] = Cpu::Instruction{"RES 7, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.L); }};
    ext_instructions[0xBE] = Cpu::Instruction{"RES 7, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_7); }};
    ext_instructions[0xBF] = Cpu::Instruction{"RES 7, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.A); }};

    ext_instructions[0xC0] = Cpu::Instruction{"SET 0, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.B); }};
    ext_instructions[0xC1] = Cpu::Instruction{"SET 0, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.C); }};
    ext_instructions[0xC2] = Cpu::Instruction{"SET 0, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.D); }};
    ext_instructions[0xC3] = Cpu::Instruction{"SET 0, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.E); }};
    ext_instructions[0xC4] = Cpu::Instruction{"SET 0, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.H); }};
    ext_instructions[0xC5] = Cpu::Instruction{"SET 0, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.L); }};
    ext_instructions[0xC6] = Cpu::Instruction{"SET 0, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_0); }};
    ext_instructions[0xC7] = Cpu::Instruction{"SET 0, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.A); }};
    ext_instructions[0xC8] = Cpu::Instruction{"SET 1, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.B); }};
    ext_instructions[0xC9] = Cpu::Instruction{"SET 1, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.C); }};
    ext_instructions[0xCA] = Cpu::Instruction{"SET 1, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.D); }};
    ext_instructions[0xCB] = Cpu::Instruction{"SET 1, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.E); }};
    ext_instructions[0xCC] = Cpu::Instruction{"SET 1, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.H); }};
    ext_instructions[0xCD] = Cpu::Instruction{"SET 1, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.L); }};
    ext_instructions[0xCE] = Cpu::Instruction{"SET 1, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_1); }};
    ext_instructions[0xCF] = Cpu::Instruction{"SET 1, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.A); }};

    ext_instructions[0xD0] = Cpu::Instruction{"SET 2, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.B); }};
    ext_instructions[0xD1] = Cpu::Instruction{"SET 2, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.C); }};
    ext_instructions[0xD2] = Cpu::Instruction{"SET 2, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.D); }};
    ext_instructions[0xD3] = Cpu::Instruction{"SET 2, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.E); }};
    ext_instructions[0xD4] = Cpu::Instruction{"SET 2, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.H); }};
    ext_instructions[0xD5] = Cpu::Instruction{"SET 2, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.L); }};
    ext_instructions[0xD6] = Cpu::Instruction{"SET 2, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_2); }};
    ext_instructions[0xD7] = Cpu::Instruction{"SET 2, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.A); }};
    ext_instructions[0xD8] = Cpu::Instruction{"SET 3, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.B); }};
    ext_instructions[0xD9] = Cpu::Instruction{"SET 3, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.C); }};
    ext_instructions[0xDA] = Cpu::Instruction{"SET 3, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.D); }};
    ext_instructions[0xDB] = Cpu::Instruction{"SET 3, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.E); }};
    ext_instructions[0xDC] = Cpu::Instruction{"SET 3, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.H); }};
    ext_instructions[0xDD] = Cpu::Instruction{"SET 3, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.L); }};
    ext_instructions[0xDE] = Cpu::Instruction{"SET 3, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_3); }};
    ext_instructions[0xDF] = Cpu::Instruction{"SET 3, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.A); }};

    ext_instructions[0xE0] = Cpu::Instruction{"SET 4, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.B); }};
    ext_instructions[0xE1] = Cpu::Instruction{"SET 4, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.C); }};
    ext_instructions[0xE2] = Cpu::Instruction{"SET 4, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.D); }};
    ext_instructions[0xE3] = Cpu::Instruction{"SET 4, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.E); }};
    ext_instructions[0xE4] = Cpu::Instruction{"SET 4, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.H); }};
    ext_instructions[0xE5] = Cpu::Instruction{"SET 4, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.L); }};
    ext_instructions[0xE6] = Cpu::Instruction{"SET 4, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_4); }};
    ext_instructions[0xE7] = Cpu::Instruction{"SET 4, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.A); }};
    ext_instructions[0xE8] = Cpu::Instruction{"SET 5, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.B); }};
    ext_instructions[0xE9] = Cpu::Instruction{"SET 5, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.C); }};
    ext_instructions[0xEA] = Cpu::Instruction{"SET 5, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.D); }};
    ext_instructions[0xEB] = Cpu::Instruction{"SET 5, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.E); }};
    ext_instructions[0xEC] = Cpu::Instruction{"SET 5, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.H); }};
    ext_instructions[0xED] = Cpu::Instruction{"SET 5, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.L); }};
    ext_instructions[0xEE] = Cpu::Instruction{"SET 5, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_5); }};
    ext_instructions[0xEF] = Cpu::Instruction{"SET 5, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.A); }};

    ext_instructions[0xF0] = Cpu::Instruction{"SET 6, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.B); }};
    ext_instructions[0xF1] = Cpu::Instruction{"SET 6, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.C); }};
    ext_instructions[0xF2] = Cpu::Instruction{"SET 6, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.D); }};
    ext_instructions[0xF3] = Cpu::Instruction{"SET 6, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.E); }};
    ext_instructions[0xF4] = Cpu::Instruction{"SET 6, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.H); }};
    ext_instructions[0xF5] = Cpu::Instruction{"SET 6, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.L); }};
    ext_instructions[0xF6] = Cpu::Instruction{"SET 6, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_6); }};
    ext_instructions[0xF7] = Cpu::Instruction{"SET 6, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.A); }};
    ext_instructions[0xF8] = Cpu::Instruction{"SET 7, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.B); }};
    ext_instructions[0xF9] = Cpu::Instruction{"SET 7, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.C); }};
    ext_instructions[0xFA] = Cpu::Instruction{"SET 7, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.D); }};
    ext_instructions[0xFB] = Cpu::Instruction{"SET 7, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.E); }};
    ext_instructions[0xFC] = Cpu::Instruction{"SET 7, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.H); }};
    ext_instructions[0xFD] = Cpu::Instruction{"SET 7, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.L); }};
    ext_instructions[0xFE] = Cpu::Instruction{"SET 7, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_7); }};
    ext_instructions[0xFF] = Cpu::Instruction{"SET 7, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.A); }};
}