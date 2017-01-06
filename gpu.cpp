#include <thread>
#include <chrono>

#include "gpu.h"
#include "mem.h"
#include "window.h"

#define INT_HBLANK 0x04
#define INT_VBLANK 0x10
#define INT_OAM    0x20
#define INT_LYC    0x40

#define MODE_MASK 0x03

#define STAT_LYC 0x04

#define CTRL_ENABLE 0x80

#define MODE_OAM 2
#define MODE_VRAM 3
#define MODE_HBLANK 0
#define MODE_VBLANK 1

#define FLAG_IF_VBLANK 0x01
#define FLAG_IF_LCD    0x02

using namespace std;

void Gpu::set_status(uint8_t mode) {
	*MEM.LCD_STAT &= ~MODE_MASK;
	*MEM.LCD_STAT |= mode;
}

// refresh every 70224 cycles
void Gpu::update(unsigned tclock) {
	bool disable = !(*MEM.LCD_CTRL & CTRL_ENABLE);

	if (!enabled) {
		if (!disable) {
			// LCD is turned ON
			clk = 0;
			enabled = true;
		} else {
			// LCD is OFF
			clk += tclock;
			if (clk >= 70224) {
				clk -= 70224;
				this_thread::sleep_for(chrono::milliseconds(16));
			}
			return;
		}
	} else if (disable) {
		// LCD is turned OFF
		clk = 0;
		enabled = false;
		*MEM.SCAN_LN = 0;
		set_status(MODE_HBLANK);
		return;		
	} 

	clk += tclock;
	static unsigned frameclock = 0;
	frameclock += tclock;
	switch (*MEM.LCD_STAT & MODE_MASK) {
		case (MODE_OAM):
			if (clk >= 80) {
				clk -= 80;
				set_status(MODE_VRAM);
			}
			break;
		case (MODE_VRAM):
			if (clk >= 172) {
				clk -= 172;
				set_status(MODE_HBLANK);
				WINDOW.render_buffer_line();
			}
			break;
		case (MODE_HBLANK):
			if (clk >= 204) {
				clk -= 204;
				*MEM.SCAN_LN += 1;
				if (*MEM.SCAN_LN == 144) {
					*MEM.IF |= FLAG_IF_VBLANK;
					set_status(MODE_VBLANK);
					WINDOW.draw_buffer();
				} else {
					set_status(MODE_OAM);
				}
			}
			break;
		case (MODE_VBLANK):
			if (clk >= 456) {
				clk -= 456;
				if (*MEM.SCAN_LN == 152) {
					*MEM.SCAN_LN = 0;
				} else if (*MEM.SCAN_LN == 0) {
					set_status(MODE_OAM);
				} else {
					*MEM.SCAN_LN += 1;	
				}
			}
			break;
	}

	if (*MEM.SCAN_LN == *MEM.LN_CMP) {
		*MEM.LCD_STAT |= STAT_LYC;
	}	else {
		*MEM.LCD_STAT &= ~STAT_LYC;
	}

	// Trigger LCD interrupt
	if (((*MEM.LCD_STAT & STAT_LYC) && (*MEM.LCD_STAT & INT_LYC)) ||
		  (((*MEM.LCD_STAT & MODE_MASK) == MODE_OAM) && (*MEM.LCD_STAT & INT_OAM)) ||
		  (((*MEM.LCD_STAT & MODE_MASK) == MODE_VBLANK) && (*MEM.LCD_STAT & INT_VBLANK)) ||
		  (((*MEM.LCD_STAT & MODE_MASK) == MODE_HBLANK) && (*MEM.LCD_STAT & INT_HBLANK))) {
		*MEM.IF |= FLAG_IF_LCD;
	}
}
