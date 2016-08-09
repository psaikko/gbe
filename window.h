#pragma once

#include <string.h>

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

typedef struct {
  GLFWwindow* game_window;
  GLFWwindow* tilemap_window;
  GLFWwindow* tileset_window;

  uint8_t game_buffer[WINDOW_H * WINDOW_W * 3];
  uint8_t tilemap_buffer[TILEMAP_WINDOW_H * TILEMAP_WINDOW_W * 3];
  uint8_t tileset_buffer[TILESET_WINDOW_H * TILESET_WINDOW_W * 3];

  void draw_pixel(uint8_t *addr, uint8_t color_id) {
  	assert(color_id <= 3);
  	switch (color_id) {
  		case 0:
  			memset(addr, 255, 3);
  			break;
  		case 1:
  			memset(addr, 192, 3);
  			break;
  		case 2:
  			memset(addr, 96, 3);
  			break;
  		case 3:
  			memset(addr, 0, 3);
  			break;
  	}
  }

  void render_buffer_line() {
  	uint8_t *MAP = (*MEM.GPU_CTRL & FLAG_GPU_BG_TM) ? MEM.TILEMAP1 : MEM.TILEMAP0;
  	uint8_t *SET = (*MEM.GPU_CTRL & FLAG_GPU_BG_TS) ? MEM.TILESET1 : MEM.TILESET0;

  	uint8_t scrl_x = *MEM.SCRL_X;
  	uint8_t scrl_y = *MEM.SCRL_Y;

  	uint8_t window_y    = *MEM.SCAN_LN;
  	uint8_t map_pixel_y = window_y + scrl_y;
  	uint8_t map_tile_y  = map_pixel_y / TILE_H;
  	uint8_t tile_y      = map_pixel_y % TILE_H;
  	for (uint8_t window_x = 0; window_x < WINDOW_W; ++window_x) {
  		uint8_t map_pixel_x = window_x + scrl_x;
  		uint8_t map_tile_x  = map_pixel_x / TILE_W;
  		uint8_t tile_x      = map_pixel_x % TILE_W;

  		// get pixel (tile_x, tile_y) from map tile (map_tile_x, map_tile_y)
  		// and draw it to (window_x, window_y)
  		uint8_t tile_id = MAP[map_tile_x + map_tile_y * TILEMAP_H];
  		uint8_t *tile = &SET[tile_id * 16];

  		uint8_t color_id = get_tile_pixel(tile, tile_x, tile_y);
  		color_id = apply_palette(color_id);

  		unsigned i = rgb_buffer_index(window_x, window_y, WINDOW_W, WINDOW_H);
  		draw_pixel(&game_buffer[i], color_id);
  	}
  }

  uint8_t apply_palette(uint8_t color_id) {
  	assert(color_id <= 3);
  	switch (color_id) {
  		case 0:
  			return (*MEM.BG_PLT & BG_PLT_COLOR0);
  		case 1:
  			return (*MEM.BG_PLT & BG_PLT_COLOR1) >> 2;
  		case 2:
  			return (*MEM.BG_PLT & BG_PLT_COLOR2) >> 4;
  		case 3:
  			return (*MEM.BG_PLT & BG_PLT_COLOR3) >> 6;
  	}
  }

  uint8_t get_tile_pixel(uint8_t *tile, uint8_t x, uint8_t y) {

  	assert(x <= 7);
  	assert(y <= 7);

  	uint8_t *row_y    = tile + 2*y; // tiles have 2 bytes per row
  	uint8_t bitmask_x = 0x80 >> x;

  	bool bit0 = row_y[0] & bitmask_x;
		bool bit1 = row_y[1] & bitmask_x;
		uint8_t color_id = bit0 + bit1 * 2;  
		return color_id;
  }

  unsigned rgb_buffer_index(unsigned x, unsigned y, unsigned w, unsigned h) {
  	return x*3 + (h - 1 - y)*w*3;
  }

  void render_tile(uint8_t *buffer, uint8_t *tile, uint8_t window_x, uint8_t window_y, uint16_t buffer_w, uint16_t buffer_h) {
  	for (uint8_t yoff = 0; yoff < TILE_H; yoff++) {
			for (uint8_t xoff = 0; xoff < TILE_W; xoff++) {
				uint8_t color_id = get_tile_pixel(tile, xoff, yoff);

				//color_id = apply_palette(color_id);

				unsigned i = rgb_buffer_index(window_x + xoff, window_y + yoff, buffer_w, buffer_h);
				draw_pixel(&buffer[i], color_id);
			}
		}
  }

  void render_tilemap() {
  	uint8_t *MAP = (*MEM.GPU_CTRL & FLAG_GPU_BG_TM) ? MEM.TILEMAP1 : MEM.TILEMAP0;
  	uint8_t *SET = (*MEM.GPU_CTRL & FLAG_GPU_BG_TS) ? MEM.TILESET1 : MEM.TILESET0;

  	for (uint8_t xoff = 0; xoff < TILEMAP_W; ++xoff) {
  		for (uint8_t yoff = 0; yoff < TILEMAP_H; ++yoff) {
  			uint8_t tile_id = MAP[xoff + yoff * TILEMAP_H];

  			uint8_t *tile = &SET[tile_id * 16];
  			uint8_t window_x = xoff * TILE_W;
  			uint8_t window_y = yoff * TILE_H;

  			render_tile(tilemap_buffer, tile, window_x, window_y, TILEMAP_WINDOW_W, TILEMAP_WINDOW_H);	
  		}	
  	}
  }

  void render_tileset() { 
  	uint8_t *SET = &MEM.RAW[0x8000];

  	uint16_t tile_id = 0;
  	for (uint8_t yoff = 0; yoff < 24; ++yoff) {
  		for (uint8_t xoff = 0; xoff < 16; ++xoff) {
  		
  			uint8_t *tile = &SET[tile_id * 16];
  			tile_id++;
  			uint8_t window_x = xoff * TILE_W;
  			uint8_t window_y = yoff * TILE_H;

  			render_tile(tileset_buffer, tile, window_x, window_y, TILESET_WINDOW_W, TILESET_WINDOW_H);	
  		}	
  	}	
  }

  void draw_buffer() {
  	static long frame = 0;
  	glfwMakeContextCurrent(game_window);

  	glClear( GL_COLOR_BUFFER_BIT );
    glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
    glDrawPixels(WINDOW_W, WINDOW_H, GL_RGB, GL_UNSIGNED_BYTE, game_buffer);
    glfwSwapBuffers(game_window);

    refresh_debug();
    printf("Frame #%ld\n", frame++);
  }

  void refresh_debug() {
  	render_tileset();
    draw_tileset();
    render_tilemap();
    draw_tilemap();
  }

  void draw_tilemap() {
		glfwMakeContextCurrent(tilemap_window);

  	glClear( GL_COLOR_BUFFER_BIT );
    glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
    glDrawPixels(TILEMAP_WINDOW_W, TILEMAP_WINDOW_H, GL_RGB, GL_UNSIGNED_BYTE, tilemap_buffer);
    glfwSwapBuffers(tilemap_window);  	
  }

  void draw_tileset() {
		glfwMakeContextCurrent(tileset_window);

  	glClear( GL_COLOR_BUFFER_BIT );
    glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
    glDrawPixels(TILESET_WINDOW_W, TILESET_WINDOW_H, GL_RGB, GL_UNSIGNED_BYTE, tileset_buffer);
    glfwSwapBuffers(tileset_window);  	
  }

  void init() {

  	memset(game_buffer, 0, WINDOW_H * WINDOW_W * 3);
		memset(tilemap_buffer, 0, TILEMAP_WINDOW_H * TILEMAP_WINDOW_W * 3);
  	memset(tileset_buffer, 0, TILESET_WINDOW_H * TILESET_WINDOW_W * 3);

    if (!glfwInit()) {
      printf("Failed to initialize GLFW\n");
      exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    // Open a window and create its OpenGL context
    game_window = glfwCreateWindow(WINDOW_W, WINDOW_H, "gbe", NULL, NULL);
    tilemap_window = glfwCreateWindow(TILEMAP_WINDOW_W, TILEMAP_WINDOW_H, "tilemap", NULL, NULL);
    tileset_window = glfwCreateWindow(TILESET_WINDOW_W, TILESET_WINDOW_H, "tileset", NULL, NULL);

    glfwSetInputMode(game_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwMakeContextCurrent(game_window);
    glewExperimental = GL_TRUE;
    glewInit();
  }

} gl_window;

gl_window WINDOW;
