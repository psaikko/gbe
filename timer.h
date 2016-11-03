#pragma once

#include "mem.h"
#include "reg.h"

// TCLK ticks at 4,194,304Hz
// MCLK ticks at 1,048,576

#define TIMER_DIV 0xFF04
#define TIMER_CNT 0xFF05
#define TIMER_MOD 0xFF06

#define TIMER_CTRL 0xFF07
#define TIMER_CTRL_SPD 0x03
#define TIMER_CTRL_RUN 0x04

#define TICK_4096_HZ 0   // M / 256
#define TICK_262144_HZ 1 // M / 4
#define TICK_65536_HZ 2  // M / 16
#define TICK_16384_HZ 3  // M / 64

typedef struct {

	unsigned div_clock;
	unsigned m_clock;

	void update() {
		div_clock += REG.TCLK;
		m_clock += REG.TCLK / 4;

		if (div_clock >= 256) {
			MEM.RAW[TIMER_DIV]++;
			div_clock -= 256;	
		}

		if (MEM.RAW[TIMER_CTRL] & TIMER_CTRL_RUN) {
			switch (MEM.RAW[TIMER_CTRL] & TIMER_CTRL_SPD) {
				case TICK_262144_HZ:
					if (m_clock >= 4) {
						if (MEM.RAW[TIMER_CNT] == 0xFF) *MEM.IF |= FLAG_IF_TIMER;
						MEM.RAW[TIMER_CNT]++;
						m_clock -= 4;
					}
					break;
				case TICK_65536_HZ:
					if (m_clock >= 16) {
						if (MEM.RAW[TIMER_CNT] == 0xFF) *MEM.IF |= FLAG_IF_TIMER;
						MEM.RAW[TIMER_CNT]++;
						m_clock -= 16;
					}
					break;
				case TICK_16384_HZ:
					if (m_clock >= 64) {
						if (MEM.RAW[TIMER_CNT] == 0xFF) *MEM.IF |= FLAG_IF_TIMER;
						MEM.RAW[TIMER_CNT]++;
						m_clock -= 64;
					}
					break;
				case TICK_4096_HZ:
					if (m_clock >= 256) {
						if (MEM.RAW[TIMER_CNT] == 0xFF) *MEM.IF |= FLAG_IF_TIMER;
						MEM.RAW[TIMER_CNT]++;
						m_clock -= 256;
					}
					break;
			}
		}
}
	
} timer;

timer TIMER;