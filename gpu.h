#pragma once

#include "reg.h"
#include "mem.h"
#include "window.h"
#include "interrupts.h"

#define MODE_OAM 2
#define MODE_VRAM 3
#define MODE_HBLANK 0
#define MODE_VBLANK 1

typedef struct {

	uint16_t clk;
	//uint8_t line;
	uint8_t mode : 2;

	void update() {
		clk += REG.TCLK;
		switch (mode) {
			case (MODE_OAM):
				if (clk >= 80) {
					clk = 0;
					mode = MODE_VRAM;
				}
				break;
			case (MODE_VRAM):
				if (clk >= 172) {
					clk = 0;
					mode = MODE_HBLANK;
					WINDOW.render_buffer_line();
				}
				break;
			case (MODE_HBLANK):
				if (clk >= 204) {
					clk = 0;
					*MEM.SCAN_LN += 1;
					if (*MEM.SCAN_LN == 143) {
						mode = MODE_VBLANK;
						WINDOW.draw_buffer();
					} else {
						mode = MODE_OAM;
					}
				}
				break;
			case (MODE_VBLANK):
				if (clk >= 456) {
					clk = 0;
					*MEM.IE |= FLAG_IF_VBLANK;
					*MEM.SCAN_LN += 1;
					if (*MEM.SCAN_LN == 153) {
						mode = MODE_OAM;
						*MEM.SCAN_LN = 0;
					}
				}
				break;
		}
	}
} gpu;

gpu GPU;