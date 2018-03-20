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

#define INT_HBLANK 0x04
#define INT_VBLANK 0x10
#define INT_OAM    0x20
#define INT_LYC    0x40

#define MODE_MASK 0x03

#define STAT_LYC 0x04

#define CTRL_ENABLE 0x80

#define MODE_OAM 2
#define MODE_VRAM 3
#define MODE_HBLANK 0
#define MODE_VBLANK 1

#define FLAG_IF_VBLANK 0x01
#define FLAG_IF_LCD    0x02

#define FLAG_GPU_BG     0x01
#define FLAG_GPU_SPR    0x02
#define FLAG_GPU_SPR_SZ 0x04
#define FLAG_GPU_BG_TM  0x08
#define FLAG_GPU_BG_WIN_TS  0x10
#define FLAG_GPU_WIN    0x20
#define FLAG_GPU_WIN_TM 0x40
#define FLAG_GPU_DISP   0x80

#define PLT_COLOR0 0x03
#define PLT_COLOR1 0x0C
#define PLT_COLOR2 0x30
#define PLT_COLOR3 0xC0

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