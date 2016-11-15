#pragma once

#include "mem.h"
#include "reg.h"

// TCLK ticks at 4,194,304Hz
// MCLK ticks at 1,048,576

#define TIMER_CTRL_SPD 0x03
#define TIMER_CTRL_RUN 0x04

#define TICK_4096_HZ 0   // M / 256
#define TICK_262144_HZ 1 // M / 4
#define TICK_65536_HZ 2  // M / 16
#define TICK_16384_HZ 3  // M / 64

typedef struct {

	unsigned div_clock;
	unsigned m_clock;

	void tick() {
		if (*MEM.TIMA == 0xFF) {
			*MEM.IF |= FLAG_IF_TIMER;
			*MEM.TIMA = *MEM.TMA;
		} else {
			(*MEM.TIMA)++;
		}
	}

	void update() {
		div_clock += REG.TCLK;
		m_clock += REG.TCLK / 4;

		if (div_clock >= 256) {
			(*MEM.DIV)++;
			div_clock -= 256;	
		}

		if (*MEM.TAC & TIMER_CTRL_RUN) {
			switch (*MEM.TAC & TIMER_CTRL_SPD) {
				case TICK_262144_HZ:
					if (m_clock >= 4) {
						tick();
						m_clock -= 4;
					}
					break;
				case TICK_65536_HZ:
					if (m_clock >= 16) {
						tick();
						m_clock -= 16;
					}
					break;
				case TICK_16384_HZ:
					if (m_clock >= 64) {
						tick();
						m_clock -= 64;
					}
					break;
				case TICK_4096_HZ:
					if (m_clock >= 256) {
						tick();
						m_clock -= 256;
					}
					break;
			}
		}
}
	
} timer;

timer TIMER;