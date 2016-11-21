#pragma once

#include <chrono>

class Memory;
class Window;

class Gpu {
public:
	Gpu(Memory &MemRef, Window &WindowRef, bool u) : clk(0), MEM(MemRef), WINDOW(WindowRef), 
		prev_frame(std::chrono::high_resolution_clock::now()), unlocked_frame_rate(u) {}

	void update(unsigned tclock);
	uint16_t clk;

private:
	Memory &MEM;
	Window &WINDOW;

	std::chrono::time_point<std::chrono::high_resolution_clock> prev_frame;
	bool unlocked_frame_rate;

	void set_status(uint8_t mode);
};