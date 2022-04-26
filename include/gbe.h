#pragma once

#include <array>
#include <string>

#define LCD_W 160u
#define LCD_H 144u

class Buttons;
class Sound;
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
    gbe(std::string romfile);

    // get current contents of lcd display (160 * RGB * 144 bytes)
    uint8_t *display();

    // run emulator for some clock cycles (70224 cycles per frame when LCD is enabled)
    void run(long clock_cycles);

    // run emulator until next complete frame is rendered
    void run_to_vblank();

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
