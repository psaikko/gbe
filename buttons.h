#pragma once

#include <inttypes.h>

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
