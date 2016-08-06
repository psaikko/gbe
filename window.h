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

#define TILEMAP_WINDOW_W 256
#define TILEMAP_WINDOW_H 256

#define TILEMAP_W 32
#define TILEMAP_H 32

#define TILE_W 8
#define TILE_H 8

typedef struct {
  GLFWwindow* game_window;
  GLFWwindow* tilemap_window;

  uint8_t game_buffer[WINDOW_H * WINDOW_W * 3];
  uint8_t tilemap_buffer[TILEMAP_WINDOW_H * TILEMAP_WINDOW_W * 3];

  void draw_pixel(uint8_t *addr, uint8_t color_id) {
  	// TODO: palette
  	uint8_t color;
  	switch (color_id) {
  		case 0:
  			color = 255;
  			break;
  		case 1:
  			color = 192;
  			break;
  		case 2:
  			color = 96;
  			break;
  		case 3:
  		default:
  			color = 0;
  			break;
  	}
  	addr[0] = color;
  	addr[1] = color;
  	addr[2] = color;
  }

  void render_buffer_line() {

  }

  void render_tilemap() {
  	uint8_t *MAP = (*MEM.GPU_CTRL & FLAG_GPU_BG_TM) ? MEM.TILEMAP1 : MEM.TILEMAP0;
  	uint8_t *SET = (*MEM.GPU_CTRL & FLAG_GPU_BG_TS) ? MEM.TILESET1 : MEM.TILESET0;

  	for (uint8_t xoff = 0; xoff < TILEMAP_W; ++xoff) {
  		for (uint8_t yoff = 0; yoff < TILEMAP_H; ++yoff) {
  			uint8_t tile_id = MAP[xoff + yoff * TILEMAP_H];

  			uint8_t *tile = &SET[tile_id];

  			uint8_t window_x = xoff * TILE_W * 3;
  			uint8_t window_y = yoff * TILE_H * 3;

  			for (uint8_t tyoff = 0; tyoff < TILE_H; tyoff++) {
  				uint8_t bitmask = 0x01;
  				for (uint8_t txoff = 0; txoff < TILE_W; txoff++) {
  					uint8_t color_id = (tile[0] & bitmask) | ((tile[1] & bitmask) << 1);
  					uint8_t *buffer_addr = &tilemap_buffer[(window_x + txoff)*3 + 
  																								 (window_y + tyoff) * TILEMAP_WINDOW_W * 3];
  					draw_pixel(buffer_addr, color_id);
  					bitmask <<= 1;
  				}
  				tile += 2;
  			}	
  		}	
  	}
  }

  void draw_buffer() {
  	glfwMakeContextCurrent(game_window);

  	glClear( GL_COLOR_BUFFER_BIT );
    glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
    glDrawPixels(WINDOW_W, WINDOW_H, GL_RGB, GL_UNSIGNED_BYTE, game_buffer);
    glfwSwapBuffers(game_window);

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

  void init() {

    if (!glfwInit()) {
      printf("Failed to initialize GLFW\n");
      exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    // Open a window and create its OpenGL context
    game_window = glfwCreateWindow(WINDOW_W, WINDOW_H, "gbe", NULL, NULL);
    tilemap_window = glfwCreateWindow(TILEMAP_WINDOW_W, TILEMAP_WINDOW_H, "tilemap", NULL, NULL);

    glfwSetInputMode(game_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwMakeContextCurrent(game_window);
    glewExperimental = GL_TRUE;
    glewInit();
  }

} gl_window;

gl_window WINDOW;
