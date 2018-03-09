#include <cstdio>

#include "serial.h"
#include "mem.h"

#define START 0x80
#define SPEED 0x02
#define CLOCK 0x01
#define SERIAL_INT 0x08

void SerialPortInterface::transfer() {
	// send *MEM.SB & (1 << transfer_bit) 
	transfer_bit++;
}

void SerialPortInterface::finish() {
	transfer_bit = 0;
	clock = 0;
	*MEM.SC &= ~START;
	//*MEM.IF |= SERIAL_INT;

	transfer_callback(*MEM.SB);

	// receive 0xFF when not connected
	*MEM.SB = 0xFF;
}

void SerialPortInterface::update(unsigned tclocks) {
	if (*MEM.SC & START) {

		if ((*MEM.SC & SPEED) != 0) {
			printf("[warning] cgb mode fast transfer not implemented\n");
		}
		if ((*MEM.SC & CLOCK) == 1) {
			
			clock += tclocks;

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
		} else {
			// serial port unimplemented
		}
	}
}