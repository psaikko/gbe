#pragma once

// GL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// timing
#include <chrono>
#include "gpu.h"

using namespace glm;

class Memory;
class Buttons;
class OpenAL_Output;
class Sound;
class GPU;

class Window {
public:
  Window(Memory &MemRef, Buttons &BtnRef, OpenAL_Output &ALRef, Sound &SndRef, Gpu &GPU, bool u) :
    MEM(MemRef), BTN(BtnRef), SND_OUT(ALRef), SND(SndRef), GPU(GPU), breakpoint(false), close(false),
    unlocked_frame_rate(u), window_scale(3), save_state(false), load_state(false), f5_down(false), f6_down(false),
    state({0,0})
  {
  }

  Memory &MEM;
  Buttons &BTN;
  Sound &SND;
  OpenAL_Output &SND_OUT;
  Gpu &GPU;

  GLFWwindow* game_window;

  unsigned window_scale;

  GLFWwindow* tilemap_window;
  GLFWwindow* tileset_window;

  uint8_t *window_buffer;

  bool breakpoint;
  bool save_state;
  bool load_state;
  bool f5_down;
  bool f6_down;
  bool close;

  void init();

  void draw_buffer();

  void update(unsigned tclock);

  struct {
    unsigned long sync_clk;
    unsigned long frames;
  } state;

private:

  void poll_buttons();

  void refresh_debug();

  void on_resize_game(int w, int h);

  void draw_tilemap();

  void draw_tileset();

  bool unlocked_frame_rate;

  friend std::ostream & operator << (std::ostream & out, const Window & win);
  friend std::istream & operator >> (std::istream & in, Window & win);
};
