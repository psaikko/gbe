#pragma once

#include "gpu.h"
#include "UI.h"
#include "sound.h"

class Memory;
class Buttons;
class OpenAL_Output;
class Sound;

class Console_IO : public UI {
 public:
  Console_IO(Memory &MemRef, Buttons &BtnRef, OpenAL_Output &ALRef, Sound &SndRef, Gpu &GPU) :
          MEM(MemRef), BTN(BtnRef), SND_OUT(ALRef), SND(SndRef), GPU(GPU), clock(0), UI()
  {
    SND.mute_ch1 = true;
    SND.mute_ch2 = true;
    SND.mute_ch3 = true;
    SND.mute_ch4 = true;
  }

  void update(unsigned tclock);

 private:

  Memory &MEM;
  Buttons &BTN;
  Sound &SND;
  OpenAL_Output &SND_OUT;
  Gpu &GPU;

  unsigned clock;

  void write(std::ostream & ) const {};
  void read(std::istream & ) {};
};

