#pragma once
#include <iostream>
#include <cstring>

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

class Memory;

class Gpu {
public:
	Gpu(Memory &MemRef) : state({0, false}), MEM(MemRef) {
    memset(gbe_buffer, 0, GBE_WINDOW_H * GBE_WINDOW_W * 3);
    memset(tilemap_buffer, 0, TILEMAP_WINDOW_H * 2 * TILEMAP_WINDOW_W * 3);
    memset(tileset_buffer, 0, TILESET_WINDOW_H * TILESET_WINDOW_W * 3);
  }

	void update(unsigned tclock);

  struct {
    unsigned clk;
    bool enabled;
  } state;

  uint8_t gbe_buffer[GBE_WINDOW_H * GBE_WINDOW_W * 3];
  uint8_t tilemap_buffer[TILEMAP_WINDOW_H * 2 * TILEMAP_WINDOW_W * 3];
  uint8_t tileset_buffer[TILESET_WINDOW_H * TILESET_WINDOW_W * 3];

  void render_tilemap();

  void render_tileset();

private:

  void render_buffer_line();

  void draw_pixel(uint8_t *addr, uint8_t color_id);

  uint8_t * get_tile(uint8_t tile_id, bool tileset0);

  uint8_t apply_palette(uint8_t color_id, uint8_t palette);

  uint8_t get_tile_pixel(uint8_t *tile, uint8_t x, uint8_t y);

  unsigned rgb_buffer_index(unsigned x, unsigned y, unsigned w, unsigned h);

  void render_tile(uint8_t *buffer, uint8_t *tile, unsigned lcd_x, unsigned lcd_y, unsigned buffer_w, unsigned buffer_h);

	Memory &MEM;

	void set_status(uint8_t mode);

  friend std::ostream & operator << (std::ostream & out, const Gpu & gpu);
  friend std::istream & operator >> (std::istream & in, Gpu & gpu);
};