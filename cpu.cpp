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