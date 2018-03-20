#include <thread>
#include <algorithm> // sort

#include "gpu.h"
#include "mem.h"

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

using namespace std;
using namespace std::chrono;

void Gpu::set_status(uint8_t mode) {
	*MEM.LCD_STAT &= ~MODE_MASK;
	*MEM.LCD_STAT |= mode;
}

void Gpu::render_tileset() {
  uint8_t *SET = MEM.TILESET1;

  uint16_t tile_id = 0;
  for (uint8_t yoff = 0; yoff < 24; ++yoff) {
    for (uint8_t xoff = 0; xoff < 16; ++xoff) {

      uint8_t *tile = &SET[tile_id * 16];
      tile_id++;
      uint8_t lcd_x = xoff * TILE_W;
      uint8_t lcd_y = yoff * TILE_H;

      render_tile(tileset_buffer.data(), tile, lcd_x, lcd_y, TILESET_WINDOW_W, TILESET_WINDOW_H);
    }
  }
}

inline void Gpu::draw_pixel(uint8_t *addr, uint8_t color_id) {
  assert(color_id <= 3);
  switch (color_id) {
    case COLOR_WHITE:
      memset(addr, 255, 3);
      break;
    case COLOR_GRAY1:
      memset(addr, 192, 3);
      break;
    case COLOR_GRAY2:
      memset(addr, 96, 3);
      break;
    default:
    case COLOR_BLACK:
      memset(addr, 0, 3);
      break;
  }
}

inline uint8_t * Gpu::get_tile(const uint8_t tile_id, const bool tileset1) {
  uint8_t *tile;
  if (tileset1) {
    tile = &MEM.TILESET1[tile_id * 16];
  } else {
    tile = MEM.getReadPtr(0x9000 + (int16_t)((int8_t)tile_id) * 16);
  }
  return tile;
}

void Gpu::render_buffer_line() {

  if (! (*MEM.LCD_CTRL & FLAG_GPU_DISP) ) return;

  uint8_t lcd_y = *MEM.SCAN_LN;
  assert(lcd_y < 144);

  uint8_t *BG_MAP = (*MEM.LCD_CTRL & FLAG_GPU_BG_TM) ? MEM.TILEMAP1 : MEM.TILEMAP0;

  uint8_t scrl_x = *MEM.SCRL_X;
  uint8_t scrl_y = *MEM.SCRL_Y;

  uint8_t bg_map_pixel_y = lcd_y + scrl_y;
  uint8_t bg_map_tile_y  = bg_map_pixel_y / TILE_H;
  uint8_t bg_tile_y   = bg_map_pixel_y % TILE_H;


  uint8_t *WIN_MAP = (*MEM.LCD_CTRL & FLAG_GPU_WIN_TM) ? MEM.TILEMAP1 : MEM.TILEMAP0;

  int window_x = *MEM.WIN_X - 7;
  int window_y = *MEM.WIN_Y;

  int win_map_pixel_y = lcd_y - window_y;
  int win_map_tile_y  = win_map_pixel_y / TILE_H;
  int win_tile_y  = win_map_pixel_y % TILE_H;

  int n_visible_sprites = 0;
  pair<int,int> visible_sprites[40]; // <x-coordinate, oam-index> pairs

  // precompute visible sprites
  for (int spr_id = 0; spr_id < 40; ++spr_id) {
    oam_entry spr = MEM.OAM[spr_id];
    const uint8_t spr_w = 8;
    uint8_t spr_h = (*MEM.LCD_CTRL & FLAG_GPU_SPR_SZ) ? 16 : 8;
    // x and y coords offset in memory..
    int spr_y = ((int)spr.y) - 16;
    int spr_x = ((int)spr.x) - 8;

    // does not intersect current scanline (lcd_y)
    if (lcd_y < spr_y || lcd_y >= spr_y + spr_h) continue;

    // not onscreen in x direction
    if (spr_x == -spr_w) continue;

    visible_sprites[n_visible_sprites] = { -spr_x, spr_id };
    n_visible_sprites++;
  }

  sort(visible_sprites, visible_sprites + n_visible_sprites);

  for (uint8_t lcd_x = 0; lcd_x < LCD_W; ++lcd_x) {
    uint8_t color_id = 0;
    uint8_t color = 0;

    if (*MEM.LCD_CTRL & FLAG_GPU_BG) {
      uint8_t bg_map_pixel_x = lcd_x + scrl_x;
      uint8_t bg_map_tile_x  = bg_map_pixel_x / TILE_W;
      uint8_t bg_tile_x      = bg_map_pixel_x % TILE_W;

      // get pixel (tile_x, tile_y) from map tile (bg_map_tile_x, bg_map_tile_y)
      // and draw it to (lcd_x, lcd_y)
      uint8_t bg_tile_id = BG_MAP[bg_map_tile_x + bg_map_tile_y * TILEMAP_H];

      uint8_t *bg_tile = get_tile(bg_tile_id, *MEM.LCD_CTRL & FLAG_GPU_BG_WIN_TS);

      color_id = get_tile_pixel(bg_tile, bg_tile_x, bg_tile_y);
      color = apply_palette(color_id, *MEM.BG_PLT);
    }

    if ((*MEM.LCD_CTRL & FLAG_GPU_WIN) && (win_map_pixel_y >= 0)) {
      int win_map_pixel_x = lcd_x - window_x;
      int win_map_tile_x  = win_map_pixel_x / TILE_W;
      int win_tile_x  = win_map_pixel_x % TILE_H;

      if (win_map_pixel_x >= 0) {
        uint8_t win_tile_id = WIN_MAP[win_map_tile_x + win_map_tile_y * TILEMAP_H];

        uint8_t *win_tile = get_tile(win_tile_id, *MEM.LCD_CTRL & FLAG_GPU_BG_WIN_TS);

        color_id = get_tile_pixel(win_tile, win_tile_x, win_tile_y);
        color = apply_palette(color_id, *MEM.BG_PLT);
      }
    }

    if (*MEM.LCD_CTRL & FLAG_GPU_SPR) {
      for (int spr_priority = min(n_visible_sprites, 10); spr_priority > 0; --spr_priority) {

        int spr_x  = -visible_sprites[spr_priority - 1].first;

        // does not hit current x pixel?
        if (lcd_x < spr_x) break; // all subsequent sprites have spr_x smaller than current
        if (lcd_x > spr_x + 7) continue;

        int spr_id = visible_sprites[spr_priority - 1].second;

        oam_entry spr = MEM.OAM[spr_id];
        const uint8_t spr_w = 8;
        uint8_t spr_h = (*MEM.LCD_CTRL & FLAG_GPU_SPR_SZ) ? 16 : 8;
        // x and y coords offset in memory..
        int spr_y = ((int)spr.y) - 16;

        uint8_t spr_tile_y = lcd_y - spr_y;
        if (spr.yflip) spr_tile_y = (spr_h - 1) - spr_tile_y;

        uint8_t spr_tile_x = lcd_x - spr_x;
        if (spr.xflip) spr_tile_x = (spr_w - 1) - spr_tile_x;

        // ignore lsb if double-height sprite
        uint8_t spr_tile_id = (spr_h == 16) ? spr.tile_id & ~1 : spr.tile_id;
        uint8_t *spr_tile = get_tile(spr_tile_id, true);

        if (!spr.priority || color_id == 0) {
          uint8_t spr_color_id = get_tile_pixel(spr_tile, spr_tile_x, spr_tile_y);
          if (spr_color_id == 0) continue; // sprite color 0 is transparent
          color_id = spr_color_id;
          color = apply_palette(color_id, spr.palette ? *MEM.OBJ1_PLT : *MEM.OBJ0_PLT);
        }
      }
    }

    unsigned i = rgb_buffer_index(lcd_x, lcd_y, LCD_W, LCD_H);
    draw_pixel(&write_buffer[i], color);
  }
}

inline uint8_t Gpu::apply_palette(uint8_t color_id, uint8_t palette) {
  assert(color_id <= 3);
  switch (color_id) {
    case 0:
      return (palette & PLT_COLOR0);
    case 1:
      return (palette & PLT_COLOR1) >> 2;
    case 2:
      return (palette & PLT_COLOR2) >> 4;
    default:
    case 3:
      return (palette & PLT_COLOR3) >> 6;
  }
}

inline uint8_t Gpu::get_tile_pixel(uint8_t *tile, uint8_t x, uint8_t y) {

  assert(x <= 7);
  assert(y <= ((*MEM.LCD_CTRL & FLAG_GPU_SPR_SZ) ? 15 : 7));

  uint8_t *row_y    = tile + 2*y; // tiles have 2 bytes per row
  uint8_t bitmask_x = 0x80 >> x;

  bool bit0 = row_y[0] & bitmask_x;
  bool bit1 = row_y[1] & bitmask_x;
  uint8_t color_id = bit0 + bit1 * 2;
  return color_id;
}

inline unsigned Gpu::rgb_buffer_index(unsigned x, unsigned y, unsigned w, unsigned h) {
  return x*3 + (h - 1 - y)*w*3;
}

void Gpu::render_tile(uint8_t *buffer, uint8_t *tile, unsigned lcd_x, unsigned lcd_y, unsigned buffer_w, unsigned buffer_h) {
  for (uint8_t yoff = 0; yoff < TILE_H; yoff++) {
    for (uint8_t xoff = 0; xoff < TILE_W; xoff++) {
      uint8_t color_id = get_tile_pixel(tile, xoff, yoff);

      unsigned i = rgb_buffer_index(lcd_x + xoff, lcd_y + yoff, buffer_w, buffer_h);
      draw_pixel(&buffer[i], color_id);
    }
  }
}

void Gpu::render_tilemap() {
  uint8_t *MAP = (*MEM.LCD_CTRL & FLAG_GPU_BG_TM) ? MEM.TILEMAP1 : MEM.TILEMAP0;

  for (uint8_t xoff = 0; xoff < TILEMAP_W; ++xoff) {
    for (uint8_t yoff = 0; yoff < TILEMAP_H; ++yoff) {
      uint8_t tile_id = MAP[xoff + yoff * TILEMAP_H];

      uint8_t *tile = get_tile(tile_id, *MEM.LCD_CTRL & FLAG_GPU_BG_WIN_TS);
      unsigned lcd_x = xoff * TILE_W;
      unsigned lcd_y = yoff * TILE_H;

      render_tile(tilemap_buffer.data(), tile, lcd_x, lcd_y, TILEMAP_WINDOW_W, TILEMAP_WINDOW_H*2);
    }
  }

  MAP = (*MEM.LCD_CTRL & FLAG_GPU_BG_TM) ? MEM.TILEMAP0 : MEM.TILEMAP1;

  for (uint8_t xoff = 0; xoff < TILEMAP_W; ++xoff) {
    for (uint8_t yoff = 0; yoff < TILEMAP_H; ++yoff) {
      uint8_t tile_id = MAP[xoff + yoff * TILEMAP_H];

      uint8_t *tile = get_tile(tile_id, *MEM.LCD_CTRL & FLAG_GPU_BG_WIN_TS);
      unsigned lcd_x = xoff * TILE_W;
      uint8_t lcd_y = yoff * TILE_H;

      render_tile(tilemap_buffer.data(), tile, lcd_x, lcd_y + TILEMAP_WINDOW_H, TILEMAP_WINDOW_W, TILEMAP_WINDOW_H*2);
    }
  }
}

// refresh every 70224 cycles
void Gpu::update(unsigned tclock) {
	bool disable = !(*MEM.LCD_CTRL & CTRL_ENABLE);

	if (!state.enabled) {
		if (!disable) {
			// LCD is turned ON
			state.clk = 0;
			state.enabled = true;
		} else {
			// LCD is OFF
		}
	} else if (disable) {
		// LCD is turned OFF
		state.clk = 0;
		state.enabled = false;
		*MEM.SCAN_LN = 0;
		set_status(MODE_HBLANK);
		return;
	}

	if (state.enabled) {
		state.clk += tclock;
		switch (*MEM.LCD_STAT & MODE_MASK) {
			case (MODE_OAM):
				if (state.clk >= 80) {
					state.clk -= 80;
					set_status(MODE_VRAM);
				}
				break;
			case (MODE_VRAM):
				if (state.clk >= 172) {
					state.clk -= 172;
					set_status(MODE_HBLANK);
					render_buffer_line();
				}
				break;
			case (MODE_HBLANK):
				if (state.clk >= 204) {
					state.clk -= 204;
					*MEM.SCAN_LN += 1;
					if (*MEM.SCAN_LN == 144) {
						*MEM.IF |= FLAG_IF_VBLANK;
						set_status(MODE_VBLANK);
						lcd_buffer.swap(write_buffer);
					} else {
						set_status(MODE_OAM);
					}
				}
				break;
			case (MODE_VBLANK):
				if (state.clk >= 456) {
					state.clk -= 456;
					if (*MEM.SCAN_LN == 152) {
						*MEM.SCAN_LN = 0;
					} else if (*MEM.SCAN_LN == 0) {
						set_status(MODE_OAM);
					} else {
						*MEM.SCAN_LN += 1;
					}
				}
				break;
		}

		if (*MEM.SCAN_LN == *MEM.LN_CMP) {
			*MEM.LCD_STAT |= STAT_LYC;
		}	else {
			*MEM.LCD_STAT &= ~STAT_LYC;
		}

		// Trigger LCD interrupt
		if (((*MEM.LCD_STAT & STAT_LYC) && (*MEM.LCD_STAT & INT_LYC)) ||
			  (((*MEM.LCD_STAT & MODE_MASK) == MODE_OAM) && (*MEM.LCD_STAT & INT_OAM)) ||
			  (((*MEM.LCD_STAT & MODE_MASK) == MODE_VBLANK) && (*MEM.LCD_STAT & INT_VBLANK)) ||
			  (((*MEM.LCD_STAT & MODE_MASK) == MODE_HBLANK) && (*MEM.LCD_STAT & INT_HBLANK))) {
			*MEM.IF |= FLAG_IF_LCD;
		}
	}
}

std::ostream & operator<<(std::ostream &out, const Gpu &gpu) {
	out.write(reinterpret_cast<const char*>(&gpu.state), sizeof(gpu.state));
	return out;
}

std::istream & operator>>(std::istream &in, Gpu &gpu) {
  in.read(reinterpret_cast<char*>(&gpu.state), sizeof(gpu.state));
	return in;
}

