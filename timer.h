#pragma once

class Memory;

class Timer {
  public:
    Timer(Memory &MemRef) : MEM(MemRef), div_clock(0), m_clock(0) {
    }

    void update(unsigned tclock);

  private:
    Memory &MEM;

    unsigned div_clock;
    unsigned m_clock;

    void tick();
};