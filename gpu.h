#pragma once

class Memory;
class Window;

class Gpu {
public:
	Gpu(Memory &MemRef, Window &WindowRef) : clk(0), MEM(MemRef), WINDOW(WindowRef) {}

	void update(unsigned tclock);
	uint16_t clk;

private:
	Memory &MEM;
	Window &WINDOW;

	void set_status(uint8_t mode);
};