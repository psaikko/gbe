#pragma once
#include <iostream>
#include <cstring>
#include <array>

#define LCD_W 160u
#define LCD_H 144u

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

class Gpu {
public:
	Gpu(Memory &MemRef) : state({0, false}), MEM(MemRef) {
    lcd_buffer.fill(0);
    write_buffer.fill(0);
    tilemap_buffer.fill(0);
    tilemap_buffer.fill(0);
  }

	void update(unsigned tclock);

  struct {
    unsigned clk;
    bool enabled;
  } state;

  std::array<uint8_t, LCD_H * LCD_W * 3>  lcd_buffer;
  std::array<uint8_t, TILEMAP_WINDOW_H * 2 * TILEMAP_WINDOW_W * 3> tilemap_buffer;
  std::array<uint8_t, TILESET_WINDOW_H * TILESET_WINDOW_W * 3> tileset_buffer;

  void render_tilemap();

  void render_tileset();

private:

  std::array<uint8_t, LCD_H * LCD_W * 3> write_buffer;

  void render_buffer_line();

  inline void draw_pixel(uint8_t *addr, uint8_t color_id);

  uint8_t * get_tile(uint8_t tile_id, bool tileset0);

  inline uint8_t apply_palette(uint8_t color_id, uint8_t palette);

  inline uint8_t get_tile_pixel(uint8_t *tile, uint8_t x, uint8_t y);

  inline unsigned rgb_buffer_index(unsigned x, unsigned y, unsigned w, unsigned h);

  inline void render_tile(uint8_t *buffer, uint8_t *tile, unsigned lcd_x, unsigned lcd_y, unsigned buffer_w, unsigned buffer_h);

	Memory &MEM;

	void set_status(uint8_t mode);

  friend std::ostream & operator << (std::ostream & out, const Gpu & gpu);
  friend std::istream & operator >> (std::istream & in, Gpu & gpu);
};