#pragma once

#include <chrono>
#include <thread>

#include "reg.h"
#include "mem.h"
#include "window.h"
#include "interrupts.h"

#define MODE_OAM 2
#define MODE_VRAM 3
#define MODE_HBLANK 0
#define MODE_VBLANK 1

#define INT_HBLANK 0x04
#define INT_VBLANK 0x10
#define INT_OAM    0x20
#define INT_LYC    0x40

#define MODE_MASK 0x03

#define STAT_LYC 0x04

using namespace std;

void update_lcd_status(uint8_t mode) {
	*MEM.LCD_STAT &= ~MODE_MASK;
	*MEM.LCD_STAT |= mode;
}

typedef struct {

	uint16_t clk;
	chrono::time_point<chrono::high_resolution_clock> prev_frame;
	bool unlocked_frame_rate;

	void update() {
		clk += REG.TCLK;
		switch (*MEM.LCD_STAT & MODE_MASK) {
			case (MODE_OAM):
				if (clk >= 80) {
					clk = 0;
					update_lcd_status(MODE_VRAM);
				}
				break;
			case (MODE_VRAM):
				if (clk >= 172) {
					clk = 0;
					update_lcd_status(MODE_HBLANK);
					WINDOW.render_buffer_line();
				}
				break;
			case (MODE_HBLANK):
				if (clk >= 204) {
					clk = 0;
					*MEM.SCAN_LN += 1;
					if (*MEM.SCAN_LN == 143) {
						update_lcd_status(MODE_VBLANK);
						WINDOW.draw_buffer();

						{
						using namespace std::chrono;
						auto time = high_resolution_clock::now();
						auto frame_time = time - prev_frame;
						long long frame_us = duration_cast<microseconds>(frame_time).count();

						// sleep until 16750 microseconds have passed since previous frame was drawn
						// (~ 59.7 fps)
						if (frame_us < 16750 && !unlocked_frame_rate) {
							unsigned sleep_us = 16750 - frame_us;
							this_thread::sleep_for(microseconds(sleep_us));
						}
						printf("frame %lu us\n", duration_cast<microseconds>(high_resolution_clock::now() - prev_frame).count());
						prev_frame = high_resolution_clock::now();
						}
					} else {
						update_lcd_status(MODE_OAM);
					}
				}
				break;
			case (MODE_VBLANK):
				if (clk >= 456) {
					clk = 0;
					if (*MEM.SCAN_LN == 143)
						*MEM.IF |= FLAG_IF_VBLANK;
					*MEM.SCAN_LN += 1;
					if (*MEM.SCAN_LN == 153) {
						update_lcd_status(MODE_OAM);
						*MEM.SCAN_LN = 0;
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
} gpu;

gpu GPU;