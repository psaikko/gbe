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

class Window {
public:
  Window(Memory &MemRef, Buttons &BtnRef, OpenAL_Output &ALRef, Sound &SndRef, Gpu &GPU, bool u) :
    MEM(MemRef), BTN(BtnRef), SND_OUT(ALRef), SND(SndRef), GPU(GPU),
    unlocked_frame_rate(u), game_scale(4), tileset_scale(2), tilemap_scale(1), f5_down(false), f6_down(false),
    state({0,0}), breakpoint(false), save_state(false), load_state(false), close(false)
  {
    if (!glfwInit()) {
      printf("Failed to initialize GLFW\n");
      exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    game_window_buffer = (uint8_t*)calloc(LCD_H * game_scale * LCD_W * game_scale * 3, sizeof(uint8_t));
    tilemap_window_buffer = (uint8_t*)calloc(TILEMAP_WINDOW_H * 2 * tilemap_scale * TILEMAP_WINDOW_W * tilemap_scale * 3, sizeof(uint8_t));
    tileset_window_buffer = (uint8_t*)calloc(TILESET_WINDOW_H * tileset_scale * TILESET_WINDOW_H * tileset_scale * 3, sizeof(uint8_t));

    // Open windows and create OpenGL contexts
    tilemap_window = glfwCreateWindow(TILEMAP_WINDOW_W * tilemap_scale, TILEMAP_WINDOW_H * tilemap_scale * 2, "gbe tilemap", nullptr, nullptr);
    tileset_window = glfwCreateWindow(TILESET_WINDOW_W * tileset_scale, TILESET_WINDOW_H * tileset_scale, "gbe tileset", nullptr, nullptr);
    game_window = glfwCreateWindow(LCD_W * game_scale, LCD_H * game_scale, "gbe buffer", nullptr, nullptr);

    glfwSetWindowAspectRatio(game_window, LCD_W, LCD_H);
    glfwSetWindowAspectRatio(tileset_window, TILESET_WINDOW_W, TILESET_WINDOW_H);
    glfwSetWindowAspectRatio(tilemap_window, TILEMAP_WINDOW_W, TILEMAP_WINDOW_H * 2);

    glfwSetInputMode(game_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwMakeContextCurrent(game_window);
    glewExperimental = GL_TRUE;
    glewInit();

    glfwSetWindowUserPointer(game_window, this);
    glfwSetWindowUserPointer(tileset_window, this);
    glfwSetWindowUserPointer(tilemap_window, this);

    glfwSetWindowSizeCallback(game_window, [](GLFWwindow* win, int w, int h)
      { static_cast<Window*>(glfwGetWindowUserPointer(win))->on_resize_game( w, h ); }
    );

    glfwSetWindowSizeCallback(tileset_window, [](GLFWwindow* win, int w, int h)
      { static_cast<Window*>(glfwGetWindowUserPointer(win))->on_resize_tileset( w, h ); }
    );

    glfwSetWindowSizeCallback(tilemap_window, [](GLFWwindow* win, int w, int h)
      { static_cast<Window*>(glfwGetWindowUserPointer(win))->on_resize_tilemap( w, h ); }
    );
  }

  void update(unsigned tclock);

  struct {
    unsigned long sync_clk;
    unsigned long frames;
  } state;

  bool breakpoint;
  bool save_state;
  bool load_state;
  bool close;

private:

  bool f5_down;
  bool f6_down;

  void draw_buffer();

  void poll_buttons();

  void scale_buffer(uint8_t * source, uint8_t * target, unsigned w, unsigned h, unsigned scale);

  void on_resize_game(int w, int h);

  void on_resize_tileset(int w, int h);

  void on_resize_tilemap(int w, int h);

  void draw_tilemap();

  void draw_tileset();

  bool unlocked_frame_rate;

  GLFWwindow* game_window;
  GLFWwindow* tilemap_window;
  GLFWwindow* tileset_window;

  unsigned game_scale;
  unsigned tileset_scale;
  unsigned tilemap_scale;

  uint8_t *game_window_buffer;
  uint8_t *tileset_window_buffer;
  uint8_t *tilemap_window_buffer;

  Memory &MEM;
  Buttons &BTN;
  Sound &SND;
  OpenAL_Output &SND_OUT;
  Gpu &GPU;

  friend std::ostream & operator << (std::ostream & out, const Window & win);
  friend std::istream & operator >> (std::istream & in, Window & win);
};
