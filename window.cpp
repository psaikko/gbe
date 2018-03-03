#include <cstring>
#include <iostream>
#include <thread>

#include "window.h"
#include "mem.h"
#include "buttons.h"
#include "openal_output.h"
#include "sound.h"
#include "sync.h"

void Window::poll_buttons() {

  glfwPollEvents();

  // Check if the ESC key was pressed or the window was closed
  if (glfwGetKey(game_window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
      glfwWindowShouldClose(game_window) == 1) {
    close = true;
  }

  breakpoint = glfwGetKey(game_window, GLFW_KEY_B) == GLFW_PRESS;

  BTN.state = 0;
  if (glfwGetKey(game_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    BTN.state |= KEY_LEFT;
  if (glfwGetKey(game_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    BTN.state |= KEY_RIGHT;
  if (glfwGetKey(game_window, GLFW_KEY_UP) == GLFW_PRESS)
    BTN.state |= KEY_UP;
  if (glfwGetKey(game_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    BTN.state |= KEY_DOWN;
  if (glfwGetKey(game_window, GLFW_KEY_Z) == GLFW_PRESS)
    BTN.state |= KEY_A;
  if (glfwGetKey(game_window, GLFW_KEY_X) == GLFW_PRESS)
    BTN.state |= KEY_B;
  if (glfwGetKey(game_window, GLFW_KEY_C) == GLFW_PRESS)
    BTN.state |= KEY_START;
  if (glfwGetKey(game_window, GLFW_KEY_V) == GLFW_PRESS)
    BTN.state |= KEY_SELECT;
  if (glfwGetKey(game_window, GLFW_KEY_F1) == GLFW_PRESS)
    SND.mute_ch1 = glfwGetKey(game_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  if (glfwGetKey(game_window, GLFW_KEY_F2) == GLFW_PRESS)
    SND.mute_ch2 = glfwGetKey(game_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  if (glfwGetKey(game_window, GLFW_KEY_F3) == GLFW_PRESS)
    SND.mute_ch3 = glfwGetKey(game_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  if (glfwGetKey(game_window, GLFW_KEY_F4) == GLFW_PRESS)
    SND.mute_ch4 = glfwGetKey(game_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

  if (glfwGetKey(game_window, GLFW_KEY_F5) == GLFW_PRESS && !f5_down) {
    f5_down = true;
  }
  if (glfwGetKey(game_window, GLFW_KEY_F6) == GLFW_PRESS && !f6_down) {
    f6_down = true;
  }

  if (glfwGetKey(game_window, GLFW_KEY_F5) == GLFW_RELEASE && f5_down) {
    save_state = true;
    f5_down = false;
  }
  if (glfwGetKey(game_window, GLFW_KEY_F6) == GLFW_RELEASE && f6_down) {
    load_state = true;
    f6_down = false;
  }
}

void Window::scale_buffer(uint8_t * source, uint8_t * target, unsigned w, unsigned h, unsigned scale) {
  const unsigned win_buffer_row_width = w*scale*3;
  const unsigned gbe_buffer_row_width = w*3;
  for (unsigned y = 0; y < h; ++y) {
    for (unsigned x = 0; x < gbe_buffer_row_width; x += 3) {
      memset(&target[(y * scale) * win_buffer_row_width + x * scale],
             source[y * gbe_buffer_row_width + x],
             3*scale);
    }

    for (unsigned y_i = 1; y_i < scale; ++y_i) {
      memcpy(&target[(y * scale + y_i) * win_buffer_row_width],
             &target[(y * scale) * win_buffer_row_width],
             win_buffer_row_width);
    }
  }
}

void Window::draw_buffer() {

  // copy gbe buffer to window buffer
  scale_buffer(GPU.gbe_buffer, game_window_buffer, LCD_W, LCD_H, game_scale);

  // draw
  poll_buttons();
  glfwMakeContextCurrent(game_window);
  glfwSwapInterval(0);
  glClear( GL_COLOR_BUFFER_BIT );
  glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
  glDrawPixels(LCD_W * game_scale, LCD_H * game_scale, GL_RGB, GL_UNSIGNED_BYTE, game_window_buffer);
  glfwSwapBuffers(game_window);

}

void Window::draw_tilemap() {
  scale_buffer(GPU.tilemap_buffer, tilemap_window_buffer, TILEMAP_WINDOW_W, TILEMAP_WINDOW_H * 2, tilemap_scale);

	glfwMakeContextCurrent(tilemap_window);
  glfwSwapInterval(0);
	glClear( GL_COLOR_BUFFER_BIT );
  glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
  glDrawPixels(TILEMAP_WINDOW_W * tilemap_scale, TILEMAP_WINDOW_H * 2 * tilemap_scale, GL_RGB, GL_UNSIGNED_BYTE, tilemap_window_buffer);
  glfwSwapBuffers(tilemap_window);  	
}

void Window::draw_tileset() {
  scale_buffer(GPU.tileset_buffer, tileset_window_buffer, TILESET_WINDOW_W, TILESET_WINDOW_H, tileset_scale);

	glfwMakeContextCurrent(tileset_window);
  glfwSwapInterval(0);
	glClear( GL_COLOR_BUFFER_BIT );
  glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
  glDrawPixels(TILESET_WINDOW_W * tileset_scale, TILESET_WINDOW_H * tileset_scale, GL_RGB, GL_UNSIGNED_BYTE, tileset_window_buffer);
  glfwSwapBuffers(tileset_window);  	
}

void Window::update(unsigned tclock) {
  state.sync_clk += tclock;

  // synchronize gpu to 59.7 fps
  if (state.sync_clk >= 70224 || unlocked_frame_rate)
  {
    state.sync_clk -= 70224;
    ++state.frames;

    long long expected_time_ms = state.frames * 10000 / 597;
    long long actual_time_ms = SyncTimer::get().elapsed_ms();

    long long delta_ms = expected_time_ms - actual_time_ms;

    if (delta_ms > 0) {
      //printf("[window] %lld ms frame time delta\n", delta_ms);
      this_thread::sleep_for(milliseconds(delta_ms));
    } else {
      //printf("[window] render lagging by %ld ms\n", delta_ms);
    }

    draw_buffer();

    GPU.render_tileset();
    draw_tileset();
    GPU.render_tilemap();
    draw_tilemap();
  }
}

void Window::on_resize_game(int w, int h) {

  unsigned new_scale = std::min(w / LCD_W, h / LCD_H);

  if (game_scale != new_scale) {
    free(game_window_buffer);
    game_scale = new_scale;
    game_window_buffer = (uint8_t *)
            calloc(LCD_H * game_scale * LCD_W * game_scale * 3, sizeof(uint8_t));
  }
}

void Window::on_resize_tileset(int w, int h) {

  unsigned new_scale = std::min(w / TILESET_WINDOW_W, h / TILESET_WINDOW_H);

  if (tileset_scale != new_scale) {
    free(tileset_window_buffer);
    tileset_scale = new_scale;
    tileset_window_buffer = (uint8_t *)
            calloc(TILESET_WINDOW_H * tileset_scale * TILESET_WINDOW_W * tileset_scale * 3, sizeof(uint8_t));
  }
}

void Window::on_resize_tilemap(int w, int h) {

  unsigned new_scale = std::min(w / TILEMAP_WINDOW_W, h / (TILEMAP_WINDOW_H * 2));

  if (tilemap_scale != new_scale) {
    free(tilemap_window_buffer);
    tilemap_scale = new_scale;
    tilemap_window_buffer = (uint8_t *)
            calloc(TILEMAP_WINDOW_H * tilemap_scale * TILEMAP_WINDOW_W * 2 * tilemap_scale * 3, sizeof(uint8_t));
  }
}

void Window::write(std::ostream &out) const {
  out.write(reinterpret_cast<const char*>(&state) , sizeof(state));
}

void Window::read(std::istream &in) {
  in.read(reinterpret_cast<char*>(&state) , sizeof(state));
}
