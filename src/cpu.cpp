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
    instructions[0x00] = {"NOP", 0, [](Cpu &CPU) { CPU.nop(); }};
    instructions[0x01] = {"LD BC, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.BC); }};
    instructions[0x02] = {"LD (BC), A", 0, [](Cpu &CPU) { CPU.ld_atrw_A(&CPU.REG.BC); }};
    instructions[0x03] = {"INC BC", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.BC); }};
    instructions[0x04] = {"INC B", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.B); }};
    instructions[0x05] = {"DEC B", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.B); }};
    instructions[0x06] = {"LD B, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.B); }};
    instructions[0x07] = {"RLCA", 0, [](Cpu &CPU) { CPU.rlca(); }};
    instructions[0x08] = {"LD (0x%04X), SP", 2, [](Cpu &CPU) { CPU.ld_atnn_SP(); }};
    instructions[0x09] = {"ADD HL, BC", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.BC); }};
    instructions[0x0A] = {"LD A, (BC)", 0, [](Cpu &CPU) { CPU.ld_A_atrw(&CPU.REG.BC); }};
    instructions[0x0B] = {"DEC BC", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.BC); }};
    instructions[0x0C] = {"INC C", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.C); }};
    instructions[0x0D] = {"DEC C", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.C); }};
    instructions[0x0E] = {"LD C, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.C); }};
    instructions[0x0F] = {"RRCA", 0, [](Cpu &CPU) { CPU.rrca(); }};

    instructions[0x10] = {"STOP", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0x11] = {"LD DE, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.DE); }};
    instructions[0x12] = {"LD (DE), A", 0, [](Cpu &CPU) { CPU.ld_atrw_A(&CPU.REG.DE); }};
    instructions[0x13] = {"INC DE", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.DE); }};
    instructions[0x14] = {"INC D", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.D); }};
    instructions[0x15] = {"DEC D", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.D); }};
    instructions[0x16] = {"LD D, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.D); }};
    instructions[0x17] = {"RLA", 0, [](Cpu &CPU) { CPU.rla(); }};
    instructions[0x18] = {"JR 0x%02X", 1, [](Cpu &CPU) { CPU.jr_e(); }};
    instructions[0x19] = {"ADD HL, DE", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.DE); }};
    instructions[0x1A] = {"LD A, (DE)", 0, [](Cpu &CPU) { CPU.ld_A_atrw(&CPU.REG.DE); }};
    instructions[0x1B] = {"DEC DE", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.DE); }};
    instructions[0x1C] = {"INC E", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.E); }};
    instructions[0x1D] = {"DEC E", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.E); }};
    instructions[0x1E] = {"LD E, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.E); }};
    instructions[0x1F] = {"RRA", 0, [](Cpu &CPU) { CPU.rra(); }};

    instructions[0x20] = {"JR NZ, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(!CPU.REG.FLAG_Z); }};
    instructions[0x21] = {"LD HL, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.HL); }};
    instructions[0x22] = {"LDI (HL), A", 0, [](Cpu &CPU) { CPU.ldi_atHL_A(); }};
    instructions[0x23] = {"INC HL", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.HL); }};
    instructions[0x24] = {"INC H", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.H); }};
    instructions[0x25] = {"DEC H", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.H); }};
    instructions[0x26] = {"LD H, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.H); }};
    instructions[0x27] = {"DAA", 0, [](Cpu &CPU) { CPU.daa(); }};
    instructions[0x28] = {"JR Z, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(CPU.REG.FLAG_Z); }};
    instructions[0x29] = {"ADD HL, HL", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.HL); }};
    instructions[0x2A] = {"LDI A, (HL)", 0, [](Cpu &CPU) { CPU.ldi_A_atHL(); }};
    instructions[0x2B] = {"DEC HL", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.HL); }};
    instructions[0x2C] = {"INC L", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.L); }};
    instructions[0x2D] = {"DEC L", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.L); }};
    instructions[0x2E] = {"LD L, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.L); }};
    instructions[0x2F] = {"CPL", 0, [](Cpu &CPU) { CPU.cpl(); }};

    instructions[0x30] = {"JR NC, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(!CPU.REG.FLAG_C); }};
    instructions[0x31] = {"LD SP, 0x%04X", 2, [](Cpu &CPU) { CPU.ld_rw_nn(&CPU.REG.SP); }};
    instructions[0x32] = {"LDD (HL), A", 0, [](Cpu &CPU) { CPU.ldd_atHL_A(); }};
    instructions[0x33] = {"INC SP", 0, [](Cpu &CPU) { CPU.inc_rw(&CPU.REG.SP); }};
    instructions[0x34] = {"INC (HL)", 0, [](Cpu &CPU) { CPU.inc_atHL(); }};
    instructions[0x35] = {"DEC (HL)", 0, [](Cpu &CPU) { CPU.dec_atHL(); }};
    instructions[0x36] = {"LD (HL), 0x%02X", 1, [](Cpu &CPU) { CPU.ld_atHL_n(); }};
    instructions[0x37] = {"SCF", 0, [](Cpu &CPU) { CPU.scf(); }};
    instructions[0x38] = {"JR C, 0x%02X", 1, [](Cpu &CPU) { CPU.jr_f_e(CPU.REG.FLAG_C); }};
    instructions[0x39] = {"ADD HL, SP", 0, [](Cpu &CPU) { CPU.add_hl_rw(&CPU.REG.SP); }};
    instructions[0x3A] = {"LDD A, (HL)", 0, [](Cpu &CPU) { CPU.ldd_A_atHL(); }};
    instructions[0x3B] = {"DEC SP", 0, [](Cpu &CPU) { CPU.dec_rw(&CPU.REG.SP); }};
    instructions[0x3C] = {"INC A", 0, [](Cpu &CPU) { CPU.inc_rb(&CPU.REG.A); }};
    instructions[0x3D] = {"DEC A", 0, [](Cpu &CPU) { CPU.dec_rb(&CPU.REG.A); }};
    instructions[0x3E] = {"LD A, 0x%02X", 1, [](Cpu &CPU) { CPU.ld_rb_n(&CPU.REG.A); }};
    instructions[0x3F] = {"CCF", 0, [](Cpu &CPU) { CPU.ccf(); }};

    instructions[0x40] = {"LD B, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.B); }};
    instructions[0x41] = {"LD B, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.C); }};
    instructions[0x42] = {"LD B, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.D); }};
    instructions[0x43] = {"LD B, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.E); }};
    instructions[0x44] = {"LD B, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.H); }};
    instructions[0x45] = {"LD B, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.L); }};
    instructions[0x46] = {"LD B, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.B); }};
    instructions[0x47] = {"LD B, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.B, &CPU.REG.A); }};
    instructions[0x48] = {"LD C, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.B); }};
    instructions[0x49] = {"LD C, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.C); }};
    instructions[0x4A] = {"LD C, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.D); }};
    instructions[0x4B] = {"LD C, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.E); }};
    instructions[0x4C] = {"LD C, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.H); }};
    instructions[0x4D] = {"LD C, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.L); }};
    instructions[0x4E] = {"LD C, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.C); }};
    instructions[0x4F] = {"LD C, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.C, &CPU.REG.A); }};

    instructions[0x50] = {"LD D, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.B); }};
    instructions[0x51] = {"LD D, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.C); }};
    instructions[0x52] = {"LD D, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.D); }};
    instructions[0x53] = {"LD D, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.E); }};
    instructions[0x54] = {"LD D, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.H); }};
    instructions[0x55] = {"LD D, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.L); }};
    instructions[0x56] = {"LD D, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.D); }};
    instructions[0x57] = {"LD D, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.D, &CPU.REG.A); }};
    instructions[0x58] = {"LD E, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.B); }};
    instructions[0x59] = {"LD E, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.C); }};
    instructions[0x5A] = {"LD E, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.D); }};
    instructions[0x5B] = {"LD E, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.E); }};
    instructions[0x5C] = {"LD E, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.H); }};
    instructions[0x5D] = {"LD E, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.L); }};
    instructions[0x5E] = {"LD E, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.E); }};
    instructions[0x5F] = {"LD E, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.E, &CPU.REG.A); }};

    instructions[0x60] = {"LD H, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.B); }};
    instructions[0x61] = {"LD H, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.C); }};
    instructions[0x62] = {"LD H, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.D); }};
    instructions[0x63] = {"LD H, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.E); }};
    instructions[0x64] = {"LD H, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.H); }};
    instructions[0x65] = {"LD H, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.L); }};
    instructions[0x66] = {"LD H, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.H); }};
    instructions[0x67] = {"LD H, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.H, &CPU.REG.A); }};
    instructions[0x68] = {"LD L, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.B); }};
    instructions[0x69] = {"LD L, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.C); }};
    instructions[0x6A] = {"LD L, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.D); }};
    instructions[0x6B] = {"LD L, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.E); }};
    instructions[0x6C] = {"LD L, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.H); }};
    instructions[0x6D] = {"LD L, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.L); }};
    instructions[0x6E] = {"LD L, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.L); }};
    instructions[0x6F] = {"LD L, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.L, &CPU.REG.A); }};

    instructions[0x70] = {"LD (HL), B", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.B); }};
    instructions[0x71] = {"LD (HL), C", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.C); }};
    instructions[0x72] = {"LD (HL), D", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.D); }};
    instructions[0x73] = {"LD (HL), E", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.E); }};
    instructions[0x74] = {"LD (HL), H", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.H); }};
    instructions[0x75] = {"LD (HL), L", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.L); }};
    instructions[0x76] = {"HALT", 0, [](Cpu &CPU) { CPU.halt(); }};
    instructions[0x77] = {"LD (HL), A", 0, [](Cpu &CPU) { CPU.ld_atHL_rb(&CPU.REG.A); }};
    instructions[0x78] = {"LD A, B", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.B); }};
    instructions[0x79] = {"LD A, C", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.C); }};
    instructions[0x7A] = {"LD A, D", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.D); }};
    instructions[0x7B] = {"LD A, E", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.E); }};
    instructions[0x7C] = {"LD A, H", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.H); }};
    instructions[0x7D] = {"LD A, L", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.L); }};
    instructions[0x7E] = {"LD A, (HL)", 0, [](Cpu &CPU) { CPU.ld_rb_atHL(&CPU.REG.A); }};
    instructions[0x7F] = {"LD A, A", 0, [](Cpu &CPU) { CPU.ld_rb_rb(&CPU.REG.A, &CPU.REG.A); }};

    instructions[0x80] = {"ADD A, B", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.B); }};
    instructions[0x81] = {"ADD A, C", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.C); }};
    instructions[0x82] = {"ADD A, D", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.D); }};
    instructions[0x83] = {"ADD A, E", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.E); }};
    instructions[0x84] = {"ADD A, H", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.H); }};
    instructions[0x85] = {"ADD A, L", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.L); }};
    instructions[0x86] = {"ADD A, (HL)", 0, [](Cpu &CPU) { CPU.add_A_atHL(); }};
    instructions[0x87] = {"ADD A, A", 0, [](Cpu &CPU) { CPU.add_A_rb(&CPU.REG.A); }};
    instructions[0x88] = {"ADC A, B", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.B); }};
    instructions[0x89] = {"ADC A, C", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.C); }};
    instructions[0x8A] = {"ADC A, D", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.D); }};
    instructions[0x8B] = {"ADC A, E", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.E); }};
    instructions[0x8C] = {"ADC A, H", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.H); }};
    instructions[0x8D] = {"ADC A, L", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.L); }};
    instructions[0x8E] = {"ADC A, (HL)", 0, [](Cpu &CPU) { CPU.adc_A_atHL(); }};
    instructions[0x8F] = {"ADC A, A", 0, [](Cpu &CPU) { CPU.adc_A_rb(&CPU.REG.A); }};

    instructions[0x90] = {"SUB A, B", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.B); }};
    instructions[0x91] = {"SUB A, C", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.C); }};
    instructions[0x92] = {"SUB A, D", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.D); }};
    instructions[0x93] = {"SUB A, E", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.E); }};
    instructions[0x94] = {"SUB A, H", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.H); }};
    instructions[0x95] = {"SUB A, L", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.L); }};
    instructions[0x96] = {"SUB A, (HL)", 0, [](Cpu &CPU) { CPU.sub_A_atHL(); }};
    instructions[0x97] = {"SUB A, A", 0, [](Cpu &CPU) { CPU.sub_A_rb(&CPU.REG.A); }};
    instructions[0x98] = {"SBC A, B", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.B); }};
    instructions[0x99] = {"SBC A, C", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.C); }};
    instructions[0x9A] = {"SBC A, D", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.D); }};
    instructions[0x9B] = {"SBC A, E", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.E); }};
    instructions[0x9C] = {"SBC A, H", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.H); }};
    instructions[0x9D] = {"SBC A, L", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.L); }};
    instructions[0x9E] = {"SBC A, (HL)", 0, [](Cpu &CPU) { CPU.sbc_A_atHL(); }};
    instructions[0x9F] = {"SBC A, A", 0, [](Cpu &CPU) { CPU.sbc_A_rb(&CPU.REG.A); }};

    instructions[0xA0] = {"AND B", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.B); }};
    instructions[0xA1] = {"AND C", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.C); }};
    instructions[0xA2] = {"AND D", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.D); }};
    instructions[0xA3] = {"AND E", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.E); }};
    instructions[0xA4] = {"AND H", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.H); }};
    instructions[0xA5] = {"AND L", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.L); }};
    instructions[0xA6] = {"AND (HL)", 0, [](Cpu &CPU) { CPU.and_atHL(); }};
    instructions[0xA7] = {"AND A", 0, [](Cpu &CPU) { CPU.and_rb(&CPU.REG.A); }};
    instructions[0xA8] = {"XOR B", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.B); }};
    instructions[0xA9] = {"XOR C", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.C); }};
    instructions[0xAA] = {"XOR D", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.D); }};
    instructions[0xAB] = {"XOR E", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.E); }};
    instructions[0xAC] = {"XOR H", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.H); }};
    instructions[0xAD] = {"XOR L", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.L); }};
    instructions[0xAE] = {"XOR (HL)", 0, [](Cpu &CPU) { CPU.xor_atHL(); }};
    instructions[0xAF] = {"XOR A", 0, [](Cpu &CPU) { CPU.xor_rb(&CPU.REG.A); }};

    instructions[0xB0] = {"OR B", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.B); }};
    instructions[0xB1] = {"OR C", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.C); }};
    instructions[0xB2] = {"OR D", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.D); }};
    instructions[0xB3] = {"OR E", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.E); }};
    instructions[0xB4] = {"OR H", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.H); }};
    instructions[0xB5] = {"OR L", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.L); }};
    instructions[0xB6] = {"OR (HL)", 0, [](Cpu &CPU) { CPU.or_atHL(); }};
    instructions[0xB7] = {"OR A", 0, [](Cpu &CPU) { CPU.or_rb(&CPU.REG.A); }};
    instructions[0xB8] = {"CP B", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.B); }};
    instructions[0xB9] = {"CP C", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.C); }};
    instructions[0xBA] = {"CP D", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.D); }};
    instructions[0xBB] = {"CP E", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.E); }};
    instructions[0xBC] = {"CP H", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.H); }};
    instructions[0xBD] = {"CP L", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.L); }};
    instructions[0xBE] = {"CP (HL)", 0, [](Cpu &CPU) { CPU.cp_atHL(); }};
    instructions[0xBF] = {"CP A", 0, [](Cpu &CPU) { CPU.cp_rb(&CPU.REG.A); }};

    instructions[0xC0] = {"RET NZ", 0, [](Cpu &CPU) { CPU.ret_f(!CPU.REG.FLAG_Z); }};
    instructions[0xC1] = {"POP BC", 0, [](Cpu &CPU) { CPU.pop_rw(&CPU.REG.BC); }};
    instructions[0xC2] = {"JP NZ, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(!CPU.REG.FLAG_Z); }};
    instructions[0xC3] = {"JP 0x%04X", 2, [](Cpu &CPU) { CPU.jp_nn(); }};
    instructions[0xC4] = {"CALL NZ, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(!CPU.REG.FLAG_Z); }};
    instructions[0xC5] = {"PUSH BC", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.BC); }};
    instructions[0xC6] = {"ADD A, 0x%02X", 1, [](Cpu &CPU) { CPU.add_A_n(); }};
    instructions[0xC7] = {"RST 0", 0, [](Cpu &CPU) { CPU.rst(0x00); }};
    instructions[0xC8] = {"RET Z", 0, [](Cpu &CPU) { CPU.ret_f(CPU.REG.FLAG_Z); }};
    instructions[0xC9] = {"RET", 0, [](Cpu &CPU) { CPU.ret(); }};
    instructions[0xCA] = {"JP Z, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(CPU.REG.FLAG_Z); }};
    instructions[0xCB] = {"Ext Op", 1, [](Cpu &CPU) { CPU.ext(); }};
    instructions[0xCC] = {"CALL Z, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(CPU.REG.FLAG_Z); }};
    instructions[0xCD] = {"CALL 0x%04X", 2, [](Cpu &CPU) { CPU.call_nn(); }};
    instructions[0xCE] = {"ADC A, 0x%02X", 1, [](Cpu &CPU) { CPU.adc_A_n(); }};
    instructions[0xCF] = {"RST 8", 0, [](Cpu &CPU) { CPU.rst(0x08); }};

    instructions[0xD0] = {"RET NC", 0, [](Cpu &CPU) { CPU.ret_f(!CPU.REG.FLAG_C); }};
    instructions[0xD1] = {"POP DE", 0, [](Cpu &CPU) { CPU.pop_rw(&CPU.REG.DE); }};
    instructions[0xD2] = {"JP NC, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(!CPU.REG.FLAG_C); }};
    instructions[0xD3] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xD4] = {"CALL NC, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(!CPU.REG.FLAG_C); }};
    instructions[0xD5] = {"PUSH DE", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.DE); }};
    instructions[0xD6] = {"SUB A, 0x%02X", 1, [](Cpu &CPU) { CPU.sub_A_n(); }};
    instructions[0xD7] = {"RST 10", 0, [](Cpu &CPU) { CPU.rst(0x10); }};
    instructions[0xD8] = {"RET C", 0, [](Cpu &CPU) { CPU.ret_f(CPU.REG.FLAG_C); }};
    instructions[0xD9] = {"RETI", 0, [](Cpu &CPU) { CPU.reti(); }};
    instructions[0xDA] = {"JP C, 0x%04X", 2, [](Cpu &CPU) { CPU.jp_f_nn(CPU.REG.FLAG_C); }};
    instructions[0xDB] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xDC] = {"CALL C, 0x%04X", 2, [](Cpu &CPU) { CPU.call_f_nn(CPU.REG.FLAG_C); }};
    instructions[0xDD] = {"MEM_SENTINEL", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xDE] = {"SBC A, 0x%02X", 1, [](Cpu &CPU) { CPU.sbc_A_n(); }};
    instructions[0xDF] = {"RST 18", 0, [](Cpu &CPU) { CPU.rst(0x18); }};

    instructions[0xE0] = {"LDH (0x%02X), A", 1, [](Cpu &CPU) { CPU.ldh_atn_A(); }};
    instructions[0xE1] = {"POP HL", 0, [](Cpu &CPU) { CPU.pop_rw(&CPU.REG.HL); }};
    instructions[0xE2] = {"LDH (C), A", 0, [](Cpu &CPU) { CPU.ldh_atC_A(); }};
    instructions[0xE3] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xE4] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xE5] = {"PUSH HL", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.HL); }};
    instructions[0xE6] = {"AND 0x%02X", 1, [](Cpu &CPU) { CPU.and_n(); }};
    instructions[0xE7] = {"RST 20", 0, [](Cpu &CPU) { CPU.rst(0x20); }};
    instructions[0xE8] = {"ADD SP, 0x%02X", 0, [](Cpu &CPU) { CPU.add_SP_e(); }};
    instructions[0xE9] = {"JP (HL)", 0, [](Cpu &CPU) { CPU.jp_atHL(); }};
    instructions[0xEA] = {"LD (0x%04X), A", 2, [](Cpu &CPU) { CPU.ld_atnn_A(); }};
    instructions[0xEB] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xEC] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xED] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xEE] = {"XOR 0x%02X", 1, [](Cpu &CPU) { CPU.xor_n(); }};
    instructions[0xEF] = {"RST 28", 0, [](Cpu &CPU) { CPU.rst(0x28); }};

    instructions[0xF0] = {"LDH A, (0x%02X)", 1, [](Cpu &CPU) { CPU.ldh_A_atn(); }};
    instructions[0xF1] = {"POP AF", 0, [](Cpu &CPU) { CPU.pop_AF(); }};
    instructions[0xF2] = {"LDH A, (C)", 0, [](Cpu &CPU) { CPU.ldh_A_atC(); }};
    instructions[0xF3] = {"DI", 0, [](Cpu &CPU) { CPU.di(); }};
    instructions[0xF4] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xF5] = {"PUSH AF", 0, [](Cpu &CPU) { CPU.push_rw(&CPU.REG.AF); }};
    instructions[0xF6] = {"OR 0x%02X", 1, [](Cpu &CPU) { CPU.or_n(); }};
    instructions[0xF7] = {"RST 30", 0, [](Cpu &CPU) { CPU.rst(0x30); }};
    instructions[0xF8] = {"LDHL SP, 0x%02X", 0, [](Cpu &CPU) { CPU.ld_HL_SP_e(); }};
    instructions[0xF9] = {"LD SP, HL", 0, [](Cpu &CPU) { CPU.ld_SP_HL(); }};
    instructions[0xFA] = {"LD A, (0x%04X)", 2, [](Cpu &CPU) { CPU.ld_A_atnn(); }};
    instructions[0xFB] = {"EI", 0, [](Cpu &CPU) { CPU.ei(); }};
    instructions[0xFC] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xFD] = {"XX", 0, [](Cpu &CPU) { CPU.TODO(); }};
    instructions[0xFE] = {"CP 0x%02X", 1, [](Cpu &CPU) { CPU.cp_n(); }};
    instructions[0xFF] = {"RST 38", 0, [](Cpu &CPU) { CPU.rst(0x38); }};
}

void Cpu::init_ext_instructions() {
    ext_instructions[0x00] = {"RLC B", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.B); }};
    ext_instructions[0x01] = {"RLC C", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.C); }};
    ext_instructions[0x02] = {"RLC D", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.D); }};
    ext_instructions[0x03] = {"RLC E", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.E); }};
    ext_instructions[0x04] = {"RLC H", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.H); }};
    ext_instructions[0x05] = {"RLC L", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.L); }};
    ext_instructions[0x06] = {"RLC (HL)", 0, [](Cpu &CPU) { CPU.rlc_atHL(); }};
    ext_instructions[0x07] = {"RLC A", 0, [](Cpu &CPU) { CPU.rlc_rb(&CPU.REG.A); }};
    ext_instructions[0x08] = {"RRC B", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.B); }};
    ext_instructions[0x09] = {"RRC C", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.C); }};
    ext_instructions[0x0A] = {"RRC D", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.D); }};
    ext_instructions[0x0B] = {"RRC E", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.E); }};
    ext_instructions[0x0C] = {"RRC H", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.H); }};
    ext_instructions[0x0D] = {"RRC L", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.L); }};
    ext_instructions[0x0E] = {"RRC (HL)", 0, [](Cpu &CPU) { CPU.rrc_atHL(); }};
    ext_instructions[0x0F] = {"RRC A", 0, [](Cpu &CPU) { CPU.rrc_rb(&CPU.REG.A); }};

    ext_instructions[0x10] = {"RL B", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.B); }};
    ext_instructions[0x11] = {"RL C", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.C); }};
    ext_instructions[0x12] = {"RL D", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.D); }};
    ext_instructions[0x13] = {"RL E", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.E); }};
    ext_instructions[0x14] = {"RL H", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.H); }};
    ext_instructions[0x15] = {"RL L", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.L); }};
    ext_instructions[0x16] = {"RL (HL)", 0, [](Cpu &CPU) { CPU.rl_atHL(); }};
    ext_instructions[0x17] = {"RL A", 0, [](Cpu &CPU) { CPU.rl_rb(&CPU.REG.A); }};
    ext_instructions[0x18] = {"RR B", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.B); }};
    ext_instructions[0x19] = {"RR C", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.C); }};
    ext_instructions[0x1A] = {"RR D", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.D); }};
    ext_instructions[0x1B] = {"RR E", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.E); }};
    ext_instructions[0x1C] = {"RR H", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.H); }};
    ext_instructions[0x1D] = {"RR L", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.L); }};
    ext_instructions[0x1E] = {"RR (HL)", 0, [](Cpu &CPU) { CPU.rr_atHL(); }};
    ext_instructions[0x1F] = {"RR A", 0, [](Cpu &CPU) { CPU.rr_rb(&CPU.REG.A); }};

    ext_instructions[0x20] = {"SLA B", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.B); }};
    ext_instructions[0x21] = {"SLA C", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.C); }};
    ext_instructions[0x22] = {"SLA D", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.D); }};
    ext_instructions[0x23] = {"SLA E", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.E); }};
    ext_instructions[0x24] = {"SLA H", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.H); }};
    ext_instructions[0x25] = {"SLA L", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.L); }};
    ext_instructions[0x26] = {"SLA (HL)", 0, [](Cpu &CPU) { CPU.sla_atHL(); }};
    ext_instructions[0x27] = {"SLA A", 0, [](Cpu &CPU) { CPU.sla_rb(&CPU.REG.A); }};
    ext_instructions[0x28] = {"SRA B", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.B); }};
    ext_instructions[0x29] = {"SRA C", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.C); }};
    ext_instructions[0x2A] = {"SRA D", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.D); }};
    ext_instructions[0x2B] = {"SRA E", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.E); }};
    ext_instructions[0x2C] = {"SRA H", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.H); }};
    ext_instructions[0x2D] = {"SRA L", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.L); }};
    ext_instructions[0x2E] = {"SRA (HL)", 0, [](Cpu &CPU) { CPU.sra_atHL(); }};
    ext_instructions[0x2F] = {"SRA A", 0, [](Cpu &CPU) { CPU.sra_rb(&CPU.REG.A); }};

    ext_instructions[0x30] = {"SWAP B", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.B); }};
    ext_instructions[0x31] = {"SWAP C", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.C); }};
    ext_instructions[0x32] = {"SWAP D", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.D); }};
    ext_instructions[0x33] = {"SWAP E", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.E); }};
    ext_instructions[0x34] = {"SWAP H", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.H); }};
    ext_instructions[0x35] = {"SWAP L", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.L); }};
    ext_instructions[0x36] = {"SWAP (HL)", 0, [](Cpu &CPU) { CPU.swap_atHL(); }};
    ext_instructions[0x37] = {"SWAP A", 0, [](Cpu &CPU) { CPU.swap_rb(&CPU.REG.A); }};
    ext_instructions[0x38] = {"SRL B", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.B); }};
    ext_instructions[0x39] = {"SRL C", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.C); }};
    ext_instructions[0x3A] = {"SRL D", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.D); }};
    ext_instructions[0x3B] = {"SRL E", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.E); }};
    ext_instructions[0x3C] = {"SRL H", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.H); }};
    ext_instructions[0x3D] = {"SRL L", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.L); }};
    ext_instructions[0x3E] = {"SRL (HL)", 0, [](Cpu &CPU) { CPU.srl_atHL(); }};
    ext_instructions[0x3F] = {"SRL A", 0, [](Cpu &CPU) { CPU.srl_rb(&CPU.REG.A); }};

    ext_instructions[0x40] = {"BIT 0, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.B); }};
    ext_instructions[0x41] = {"BIT 0, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.C); }};
    ext_instructions[0x42] = {"BIT 0, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.D); }};
    ext_instructions[0x43] = {"BIT 0, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.E); }};
    ext_instructions[0x44] = {"BIT 0, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.H); }};
    ext_instructions[0x45] = {"BIT 0, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.L); }};
    ext_instructions[0x46] = {"BIT 0, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_0); }};
    ext_instructions[0x47] = {"BIT 0, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_0, &CPU.REG.A); }};
    ext_instructions[0x48] = {"BIT 1, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.B); }};
    ext_instructions[0x49] = {"BIT 1, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.C); }};
    ext_instructions[0x4A] = {"BIT 1, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.D); }};
    ext_instructions[0x4B] = {"BIT 1, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.E); }};
    ext_instructions[0x4C] = {"BIT 1, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.H); }};
    ext_instructions[0x4D] = {"BIT 1, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.L); }};
    ext_instructions[0x4E] = {"BIT 1, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_1); }};
    ext_instructions[0x4F] = {"BIT 1, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_1, &CPU.REG.A); }};

    ext_instructions[0x50] = {"BIT 2, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.B); }};
    ext_instructions[0x51] = {"BIT 2, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.C); }};
    ext_instructions[0x52] = {"BIT 2, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.D); }};
    ext_instructions[0x53] = {"BIT 2, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.E); }};
    ext_instructions[0x54] = {"BIT 2, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.H); }};
    ext_instructions[0x55] = {"BIT 2, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.L); }};
    ext_instructions[0x56] = {"BIT 2, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_2); }};
    ext_instructions[0x57] = {"BIT 2, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_2, &CPU.REG.A); }};
    ext_instructions[0x58] = {"BIT 3, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.B); }};
    ext_instructions[0x59] = {"BIT 3, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.C); }};
    ext_instructions[0x5A] = {"BIT 3, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.D); }};
    ext_instructions[0x5B] = {"BIT 3, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.E); }};
    ext_instructions[0x5C] = {"BIT 3, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.H); }};
    ext_instructions[0x5D] = {"BIT 3, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.L); }};
    ext_instructions[0x5E] = {"BIT 3, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_3); }};
    ext_instructions[0x5F] = {"BIT 3, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_3, &CPU.REG.A); }};

    ext_instructions[0x60] = {"BIT 4, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.B); }};
    ext_instructions[0x61] = {"BIT 4, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.C); }};
    ext_instructions[0x62] = {"BIT 4, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.D); }};
    ext_instructions[0x63] = {"BIT 4, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.E); }};
    ext_instructions[0x64] = {"BIT 4, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.H); }};
    ext_instructions[0x65] = {"BIT 4, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.L); }};
    ext_instructions[0x66] = {"BIT 4, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_4); }};
    ext_instructions[0x67] = {"BIT 4, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_4, &CPU.REG.A); }};
    ext_instructions[0x68] = {"BIT 5, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.B); }};
    ext_instructions[0x69] = {"BIT 5, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.C); }};
    ext_instructions[0x6A] = {"BIT 5, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.D); }};
    ext_instructions[0x6B] = {"BIT 5, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.E); }};
    ext_instructions[0x6C] = {"BIT 5, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.H); }};
    ext_instructions[0x6D] = {"BIT 5, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.L); }};
    ext_instructions[0x6E] = {"BIT 5, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_5); }};
    ext_instructions[0x6F] = {"BIT 5, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_5, &CPU.REG.A); }};

    ext_instructions[0x70] = {"BIT 6, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.B); }};
    ext_instructions[0x71] = {"BIT 6, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.C); }};
    ext_instructions[0x72] = {"BIT 6, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.D); }};
    ext_instructions[0x73] = {"BIT 6, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.E); }};
    ext_instructions[0x74] = {"BIT 6, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.H); }};
    ext_instructions[0x75] = {"BIT 6, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.L); }};
    ext_instructions[0x76] = {"BIT 6, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_6); }};
    ext_instructions[0x77] = {"BIT 6, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_6, &CPU.REG.A); }};
    ext_instructions[0x78] = {"BIT 7, B", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.B); }};
    ext_instructions[0x79] = {"BIT 7, C", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.C); }};
    ext_instructions[0x7A] = {"BIT 7, D", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.D); }};
    ext_instructions[0x7B] = {"BIT 7, E", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.E); }};
    ext_instructions[0x7C] = {"BIT 7, H", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.H); }};
    ext_instructions[0x7D] = {"BIT 7, L", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.L); }};
    ext_instructions[0x7E] = {"BIT 7, (HL)", 0, [](Cpu &CPU) { CPU.bit_i_atHL(BIT_7); }};
    ext_instructions[0x7F] = {"BIT 7, A", 0, [](Cpu &CPU) { CPU.bit_i_rb(BIT_7, &CPU.REG.A); }};

    ext_instructions[0x80] = {"RES 0, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.B); }};
    ext_instructions[0x81] = {"RES 0, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.C); }};
    ext_instructions[0x82] = {"RES 0, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.D); }};
    ext_instructions[0x83] = {"RES 0, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.E); }};
    ext_instructions[0x84] = {"RES 0, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.H); }};
    ext_instructions[0x85] = {"RES 0, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.L); }};
    ext_instructions[0x86] = {"RES 0, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_0); }};
    ext_instructions[0x87] = {"RES 0, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_0, &CPU.REG.A); }};
    ext_instructions[0x88] = {"RES 1, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.B); }};
    ext_instructions[0x89] = {"RES 1, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.C); }};
    ext_instructions[0x8A] = {"RES 1, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.D); }};
    ext_instructions[0x8B] = {"RES 1, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.E); }};
    ext_instructions[0x8C] = {"RES 1, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.H); }};
    ext_instructions[0x8D] = {"RES 1, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.L); }};
    ext_instructions[0x8E] = {"RES 1, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_1); }};
    ext_instructions[0x8F] = {"RES 1, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_1, &CPU.REG.A); }};

    ext_instructions[0x90] = {"RES 2, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.B); }};
    ext_instructions[0x91] = {"RES 2, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.C); }};
    ext_instructions[0x92] = {"RES 2, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.D); }};
    ext_instructions[0x93] = {"RES 2, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.E); }};
    ext_instructions[0x94] = {"RES 2, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.H); }};
    ext_instructions[0x95] = {"RES 2, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.L); }};
    ext_instructions[0x96] = {"RES 2, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_2); }};
    ext_instructions[0x97] = {"RES 2, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_2, &CPU.REG.A); }};
    ext_instructions[0x98] = {"RES 3, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.B); }};
    ext_instructions[0x99] = {"RES 3, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.C); }};
    ext_instructions[0x9A] = {"RES 3, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.D); }};
    ext_instructions[0x9B] = {"RES 3, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.E); }};
    ext_instructions[0x9C] = {"RES 3, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.H); }};
    ext_instructions[0x9D] = {"RES 3, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.L); }};
    ext_instructions[0x9E] = {"RES 3, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_3); }};
    ext_instructions[0x9F] = {"RES 3, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_3, &CPU.REG.A); }};

    ext_instructions[0xA0] = {"RES 4, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.B); }};
    ext_instructions[0xA1] = {"RES 4, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.C); }};
    ext_instructions[0xA2] = {"RES 4, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.D); }};
    ext_instructions[0xA3] = {"RES 4, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.E); }};
    ext_instructions[0xA4] = {"RES 4, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.H); }};
    ext_instructions[0xA5] = {"RES 4, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.L); }};
    ext_instructions[0xA6] = {"RES 4, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_4); }};
    ext_instructions[0xA7] = {"RES 4, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_4, &CPU.REG.A); }};
    ext_instructions[0xA8] = {"RES 5, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.B); }};
    ext_instructions[0xA9] = {"RES 5, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.C); }};
    ext_instructions[0xAA] = {"RES 5, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.D); }};
    ext_instructions[0xAB] = {"RES 5, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.E); }};
    ext_instructions[0xAC] = {"RES 5, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.H); }};
    ext_instructions[0xAD] = {"RES 5, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.L); }};
    ext_instructions[0xAE] = {"RES 5, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_5); }};
    ext_instructions[0xAF] = {"RES 5, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_5, &CPU.REG.A); }};

    ext_instructions[0xB0] = {"RES 6, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.B); }};
    ext_instructions[0xB1] = {"RES 6, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.C); }};
    ext_instructions[0xB2] = {"RES 6, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.D); }};
    ext_instructions[0xB3] = {"RES 6, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.E); }};
    ext_instructions[0xB4] = {"RES 6, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.H); }};
    ext_instructions[0xB5] = {"RES 6, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.L); }};
    ext_instructions[0xB6] = {"RES 6, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_6); }};
    ext_instructions[0xB7] = {"RES 6, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_6, &CPU.REG.A); }};
    ext_instructions[0xB8] = {"RES 7, B", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.B); }};
    ext_instructions[0xB9] = {"RES 7, C", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.C); }};
    ext_instructions[0xBA] = {"RES 7, D", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.D); }};
    ext_instructions[0xBB] = {"RES 7, E", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.E); }};
    ext_instructions[0xBC] = {"RES 7, H", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.H); }};
    ext_instructions[0xBD] = {"RES 7, L", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.L); }};
    ext_instructions[0xBE] = {"RES 7, (HL)", 0, [](Cpu &CPU) { CPU.res_i_atHL(BIT_7); }};
    ext_instructions[0xBF] = {"RES 7, A", 0, [](Cpu &CPU) { CPU.res_i_rb(BIT_7, &CPU.REG.A); }};

    ext_instructions[0xC0] = {"SET 0, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.B); }};
    ext_instructions[0xC1] = {"SET 0, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.C); }};
    ext_instructions[0xC2] = {"SET 0, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.D); }};
    ext_instructions[0xC3] = {"SET 0, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.E); }};
    ext_instructions[0xC4] = {"SET 0, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.H); }};
    ext_instructions[0xC5] = {"SET 0, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.L); }};
    ext_instructions[0xC6] = {"SET 0, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_0); }};
    ext_instructions[0xC7] = {"SET 0, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_0, &CPU.REG.A); }};
    ext_instructions[0xC8] = {"SET 1, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.B); }};
    ext_instructions[0xC9] = {"SET 1, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.C); }};
    ext_instructions[0xCA] = {"SET 1, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.D); }};
    ext_instructions[0xCB] = {"SET 1, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.E); }};
    ext_instructions[0xCC] = {"SET 1, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.H); }};
    ext_instructions[0xCD] = {"SET 1, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.L); }};
    ext_instructions[0xCE] = {"SET 1, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_1); }};
    ext_instructions[0xCF] = {"SET 1, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_1, &CPU.REG.A); }};

    ext_instructions[0xD0] = {"SET 2, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.B); }};
    ext_instructions[0xD1] = {"SET 2, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.C); }};
    ext_instructions[0xD2] = {"SET 2, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.D); }};
    ext_instructions[0xD3] = {"SET 2, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.E); }};
    ext_instructions[0xD4] = {"SET 2, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.H); }};
    ext_instructions[0xD5] = {"SET 2, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.L); }};
    ext_instructions[0xD6] = {"SET 2, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_2); }};
    ext_instructions[0xD7] = {"SET 2, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_2, &CPU.REG.A); }};
    ext_instructions[0xD8] = {"SET 3, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.B); }};
    ext_instructions[0xD9] = {"SET 3, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.C); }};
    ext_instructions[0xDA] = {"SET 3, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.D); }};
    ext_instructions[0xDB] = {"SET 3, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.E); }};
    ext_instructions[0xDC] = {"SET 3, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.H); }};
    ext_instructions[0xDD] = {"SET 3, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.L); }};
    ext_instructions[0xDE] = {"SET 3, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_3); }};
    ext_instructions[0xDF] = {"SET 3, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_3, &CPU.REG.A); }};

    ext_instructions[0xE0] = {"SET 4, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.B); }};
    ext_instructions[0xE1] = {"SET 4, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.C); }};
    ext_instructions[0xE2] = {"SET 4, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.D); }};
    ext_instructions[0xE3] = {"SET 4, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.E); }};
    ext_instructions[0xE4] = {"SET 4, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.H); }};
    ext_instructions[0xE5] = {"SET 4, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.L); }};
    ext_instructions[0xE6] = {"SET 4, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_4); }};
    ext_instructions[0xE7] = {"SET 4, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_4, &CPU.REG.A); }};
    ext_instructions[0xE8] = {"SET 5, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.B); }};
    ext_instructions[0xE9] = {"SET 5, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.C); }};
    ext_instructions[0xEA] = {"SET 5, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.D); }};
    ext_instructions[0xEB] = {"SET 5, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.E); }};
    ext_instructions[0xEC] = {"SET 5, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.H); }};
    ext_instructions[0xED] = {"SET 5, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.L); }};
    ext_instructions[0xEE] = {"SET 5, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_5); }};
    ext_instructions[0xEF] = {"SET 5, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_5, &CPU.REG.A); }};

    ext_instructions[0xF0] = {"SET 6, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.B); }};
    ext_instructions[0xF1] = {"SET 6, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.C); }};
    ext_instructions[0xF2] = {"SET 6, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.D); }};
    ext_instructions[0xF3] = {"SET 6, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.E); }};
    ext_instructions[0xF4] = {"SET 6, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.H); }};
    ext_instructions[0xF5] = {"SET 6, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.L); }};
    ext_instructions[0xF6] = {"SET 6, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_6); }};
    ext_instructions[0xF7] = {"SET 6, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_6, &CPU.REG.A); }};
    ext_instructions[0xF8] = {"SET 7, B", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.B); }};
    ext_instructions[0xF9] = {"SET 7, C", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.C); }};
    ext_instructions[0xFA] = {"SET 7, D", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.D); }};
    ext_instructions[0xFB] = {"SET 7, E", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.E); }};
    ext_instructions[0xFC] = {"SET 7, H", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.H); }};
    ext_instructions[0xFD] = {"SET 7, L", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.L); }};
    ext_instructions[0xFE] = {"SET 7, (HL)", 0, [](Cpu &CPU) { CPU.set_i_atHL(BIT_7); }};
    ext_instructions[0xFF] = {"SET 7, A", 0, [](Cpu &CPU) { CPU.set_i_rb(BIT_7, &CPU.REG.A); }};
}