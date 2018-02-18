#pragma once

// GL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// timing
#include <chrono>

using namespace glm;

#define GBE_WINDOW_W 160u
#define GBE_WINDOW_H 144u

#define TILESET_WINDOW_W 128u
#define TILESET_WINDOW_H 192u

#define TILEMAP_WINDOW_W 256u
#define TILEMAP_WINDOW_H 256u

#define TILEMAP_W 32u
#define TILEMAP_H 32u

#define TILE_W 8u
#define TILE_H 8u

#define COLOR_WHITE 0
#define COLOR_GRAY1 1
#define COLOR_GRAY2 2
#define COLOR_BLACK 3 

class Memory;
class Buttons;
class OpenAL_Output;
class Sound;

class Window {
public:
  Window(Memory &MemRef, Buttons &BtnRef, OpenAL_Output &ALRef, Sound &SndRef, bool u) : 
    MEM(MemRef), BTN(BtnRef), SND_OUT(ALRef), SND(SndRef), breakpoint(false), close(false),
    prev_frame(std::chrono::high_resolution_clock::now()), unlocked_frame_rate(u), window_scale(3) {}

  Memory &MEM;
  Buttons &BTN;
  Sound &SND;
  OpenAL_Output &SND_OUT;

  GLFWwindow* game_window;


  unsigned window_scale;

  GLFWwindow* tilemap_window;
  GLFWwindow* tileset_window;

  uint8_t *window_buffer;

  uint8_t gbe_buffer[GBE_WINDOW_H * GBE_WINDOW_W * 3];
  uint8_t tilemap_buffer[TILEMAP_WINDOW_H * 2 * TILEMAP_WINDOW_W * 3];
  uint8_t tileset_buffer[TILESET_WINDOW_H * TILESET_WINDOW_W * 3];

  bool breakpoint;
  bool close;

  void poll_buttons();

  void debug_pixel(uint8_t *addr);

  void draw_pixel(uint8_t *addr, uint8_t color_id);

  uint8_t * get_tile(uint8_t tile_id, bool tileset0);

  void render_buffer_line();

  uint8_t apply_palette(uint8_t color_id, uint8_t palette);

  uint8_t get_tile_pixel(uint8_t *tile, uint8_t x, uint8_t y);

  unsigned rgb_buffer_index(unsigned x, unsigned y, unsigned w, unsigned h);

  void render_tile(uint8_t *buffer, uint8_t *tile, unsigned lcd_x, unsigned lcd_y, unsigned buffer_w, unsigned buffer_h);

  void render_tilemap();

  void render_tileset();

  void draw_buffer();

  void refresh_debug();

  void draw_tilemap();

  void draw_tileset();

  void on_resize_game(int w, int h);

  void init();
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> prev_frame;
  bool unlocked_frame_rate;

};
