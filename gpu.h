#pragma once
#include <iostream>

class Memory;
class Window;

class Gpu {
public:
	Gpu(Memory &MemRef, Window &WindowRef) : state({}), MEM(MemRef), WINDOW(WindowRef) {}

	void update(unsigned tclock);

  struct {
    unsigned clk;
    unsigned long sync_clk;
    unsigned long frames;
    bool enabled;
  } state;

private:
	Memory &MEM;
	Window &WINDOW;

	void set_status(uint8_t mode);

  friend std::ostream & operator << (std::ostream & out, const Gpu & gpu);
  friend std::istream & operator >> (std::istream & in, Gpu & gpu);
};