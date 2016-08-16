#pragma once
#include "reg.h"
#include "mem.h"

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

void handle_interrupts() {
	uint8_t trigger = *MEM.IE & *MEM.IF;
	if (REG.IME && trigger) {
		REG.HALT = 0;
		
		REG.IME = 0;
		if (trigger & FLAG_IF_VBLANK) {
			*MEM.IF ^= FLAG_IF_VBLANK;
			rst(ISR_VBLANK);
		} else if (trigger & FLAG_IF_LCD) {
			*MEM.IF ^= FLAG_IF_LCD;
			rst(ISR_LCD);
		} else if (trigger & FLAG_IF_TIMER) {
			*MEM.IF ^= FLAG_IF_TIMER;
			rst(ISR_TIMER);
		} else if (trigger & FLAG_IF_JOYPAD) {
			*MEM.IF ^= FLAG_IF_JOYPAD;
			rst(ISR_JOYPAD);
		} else { // if (trigger & FLAG_IF_SERIAL)
			*MEM.IF ^= FLAG_IF_SERIAL;
			rst(ISR_SERIAL);
		}
	}
}