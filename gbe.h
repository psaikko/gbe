#pragma once

#include <string>
#include <array>

#define LCD_W 160u
#define LCD_H 144u

class Buttons;
class Sound;
class OpenAL_Output;
class Cart;
class Memory;
class Registers;
class Gpu;
class Timer;
class Cpu;
class SerialPortInterface;

/*
 * Minimal interface for operating gbe
 */
class gbe {
 public:

  gbe (std::string romfile);

  // get current contents of lcd display (160 * RGB * 144 bytes)
  std::array<uint8_t, 160*3*144> display();

  // run emulator for some clock cycles (70224 cycles per frame when LCD is enabled)
  void run(long clock_cycles);

  // set button states (lasts until next input call)
  void input(bool up, bool down, bool left, bool right, bool a, bool b, bool start, bool select);

  // read memory at location addr
  uint8_t mem(uint16_t addr);

 private:

  long clock_overflow;

  Buttons *BTN;
  Sound *SND;
  Cart *CART;
  Memory *MEM;
  Registers *REG;
  Gpu *GPU;
  Timer *TIMER;
  Cpu *CPU;
  SerialPortInterface *SERIAL;
};

