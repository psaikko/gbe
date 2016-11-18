#pragma once

#include "mem.h"

#define START 0x80
#define SPEED 0x02
#define CLOCK 0x01
#define SERIAL_INT 0x08

typedef struct {

	uint8_t transfer_bit;
	unsigned clock;

	void transfer() {
		// send *MEM.SB & (1 << transfer_bit) 
		transfer_bit++;
	}

	void finish() {
		transfer_bit = 0;
		clock = 0;
		*MEM.SC &= ~START;
		*MEM.IF |= SERIAL_INT;

		// log transfer
		// printf("[serial] 0x%02X\n", *MEM.SB);
		// printf("%c", (char)(*MEM.SB));

		// receive 0xFF when not connected
		*MEM.SB = 0xFF;
	}

	void update(unsigned tclocks) {
		if (*MEM.SC & START) {

			clock += tclocks;

			if ((*MEM.SC & SPEED) != 0) {
				printf("[warning] cgb mode fast transfer not implemented\n");
			}
			if ((*MEM.SC & CLOCK) == 0) {
				printf("[error] serial transfer external clock unavailable\n");
			}

			// tclock runs at 4,194,304 Hz
			// DMG transfer 8,192 Hz
			// transfer one bit every 512 tclocks

			if (clock >= 512) {
				clock -= 512;

				transfer();

				if (transfer_bit == 8) {
					finish();
				}
			}

		}
	}

} serial_port_interface;

serial_port_interface SERIAL;