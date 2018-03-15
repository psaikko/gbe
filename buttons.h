#pragma once

#include <inttypes.h>

#define KEY_RIGHT  0x01
#define KEY_LEFT   0x02
#define KEY_UP     0x04
#define KEY_DOWN   0x08
#define KEY_A      0x10
#define KEY_B      0x20
#define KEY_START  0x80
#define KEY_SELECT 0x40

class Buttons {
public:
	Buttons() : state(0) {}

	uint8_t state;

	uint8_t dpad_state() {
		return state & 0x0F;
	}

	uint8_t key_state() {
		return state >> 4;
	}
};
