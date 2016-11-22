#pragma once

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>

using namespace glm;

#define WINDOW_W 160
#define WINDOW_H 144

#define TILESET_WINDOW_W 128
#define TILESET_WINDOW_H 192

#define TILEMAP_WINDOW_W 256
#define TILEMAP_WINDOW_H 256

#define TILEMAP_W 32
#define TILEMAP_H 32

#define TILE_W 8
#define TILE_H 8

#define COLOR_WHITE 0
#define COLOR_GRAY1 1
#define COLOR_GRAY2 2
#define COLOR_BLACK 3 

class Memory;
class Buttons;

class Window {
public:
  Window(Memory &MemRef, Buttons &BtnRef) : MEM(MemRef), BTN(BtnRef), breakpoint(false) {}

  Memory &MEM;
  Buttons &BTN;

  GLFWwindow* game_window;
  GLFWwindow* tilemap_window;
  GLFWwindow* tileset_window;

  uint8_t game_buffer[WINDOW_H * WINDOW_W * 3];
  uint8_t tilemap_buffer[TILEMAP_WINDOW_H * 2 * TILEMAP_WINDOW_W * 3];
  uint8_t tileset_buffer[TILESET_WINDOW_H * TILESET_WINDOW_W * 3];

  bool breakpoint;

  void poll_buttons();

  void debug_pixel(uint8_t *addr);

  void draw_pixel(uint8_t *addr, uint8_t color_id);

  uint8_t * get_tile(const uint8_t tile_id, const bool tileset0);

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

  void init();

};
