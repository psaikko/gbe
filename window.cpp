#include <cstring>
#include <iostream>
#include <thread>

#include "window.h"
#include "mem.h"
#include "buttons.h"
#include "openal_output.h"
#include "sound.h"
#include "sync.h"

#define KEY_RIGHT  0x01
#define KEY_LEFT   0x02
#define KEY_UP     0x04
#define KEY_DOWN   0x08
#define KEY_A      0x10
#define KEY_B      0x20
#define KEY_START  0x40
#define KEY_SELECT 0x80

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

void Window::draw_buffer() {

  // copy gbe buffer to window buffer
  const unsigned win_buffer_row_width = GBE_WINDOW_W*window_scale*3;
  const unsigned gbe_buffer_row_width = GBE_WINDOW_W*3;
  for (unsigned y = 0; y < GBE_WINDOW_H; ++y) {
    for (unsigned x = 0; x < gbe_buffer_row_width; x += 3) {
      memset(&window_buffer[(y * window_scale) * win_buffer_row_width + x * window_scale],
             GPU.gbe_buffer[y * gbe_buffer_row_width + x],
             3*window_scale);
    }

    for (unsigned y_i = 1; y_i < window_scale; ++y_i) {
      memcpy(&window_buffer[(y * window_scale + y_i) * win_buffer_row_width],
             &window_buffer[(y * window_scale) * win_buffer_row_width],
             win_buffer_row_width);
    }
  }

  // draw
  poll_buttons();
  glfwMakeContextCurrent(game_window);
  glfwSwapInterval(0);
  glClear( GL_COLOR_BUFFER_BIT );
  glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
  glDrawPixels(GBE_WINDOW_W * window_scale, GBE_WINDOW_H * window_scale, GL_RGB, GL_UNSIGNED_BYTE, window_buffer);
  glfwSwapBuffers(game_window);

  refresh_debug();
}

void Window::refresh_debug() {
  GPU.render_tileset();
  draw_tileset();
  GPU.render_tilemap();
  draw_tilemap();
}

void Window::draw_tilemap() {
	glfwMakeContextCurrent(tilemap_window);
  glfwSwapInterval(0);
	glClear( GL_COLOR_BUFFER_BIT );
  glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
  glDrawPixels(TILEMAP_WINDOW_W, TILEMAP_WINDOW_H * 2, GL_RGB, GL_UNSIGNED_BYTE, GPU.tilemap_buffer);
  glfwSwapBuffers(tilemap_window);  	
}

void Window::draw_tileset() {
	glfwMakeContextCurrent(tileset_window);
  glfwSwapInterval(0);
	glClear( GL_COLOR_BUFFER_BIT );
  glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
  glDrawPixels(TILESET_WINDOW_W, TILESET_WINDOW_H, GL_RGB, GL_UNSIGNED_BYTE, GPU.tileset_buffer);
  glfwSwapBuffers(tileset_window);  	
}

void Window::init() {
  window_buffer = (uint8_t*)calloc(GBE_WINDOW_H * window_scale * GBE_WINDOW_W * window_scale * 3, sizeof(uint8_t));

  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    exit(1);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

  // Open a window and create its OpenGL context
  tilemap_window = glfwCreateWindow(TILEMAP_WINDOW_W, TILEMAP_WINDOW_H * 2, "gbe tilemap", nullptr, nullptr);
  tileset_window = glfwCreateWindow(TILESET_WINDOW_W, TILESET_WINDOW_H, "gbe tileset", nullptr, nullptr);
  game_window = glfwCreateWindow(GBE_WINDOW_W * window_scale, GBE_WINDOW_H * window_scale, "gbe buffer", nullptr, nullptr);


  glfwSetInputMode(game_window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwMakeContextCurrent(game_window);
  glewExperimental = GL_TRUE;
  glewInit();

  glfwSetWindowUserPointer(game_window, this);

  auto func = [](GLFWwindow* win, int w, int h)
  {
    static_cast<Window*>(glfwGetWindowUserPointer(win))->on_resize_game( w, h );
  };

  glfwSetWindowSizeCallback(game_window, func);
}

void Window::update(unsigned tclock) {
  state.sync_clk += tclock;

  // synchronize gpu to 59.7 fps
  if (state.sync_clk >= 70224)
  {
    state.sync_clk -= 70224;
    ++state.frames;

    long long expected_time_ms = state.frames * 10000 / 597;
    long long actual_time_ms = SyncTimer::get().elapsed_ms();

    long long delta_ms = expected_time_ms - actual_time_ms;

    if (delta_ms > 0) {
      //printf("[gpu] %lld ms frame time delta\n", delta_ms);
      this_thread::sleep_for(milliseconds(delta_ms));
    } else {
      //printf("[gpu] render lagging by %ld ms\n", delta_ms);
    }

    draw_buffer();
  }
}

void Window::on_resize_game(int w, int h) {

  unsigned new_scale = std::min(w / GBE_WINDOW_W, h / GBE_WINDOW_H);

  if (window_scale != new_scale) {
    free(window_buffer);
    window_scale = new_scale;
    window_buffer = (uint8_t *)
            calloc(GBE_WINDOW_H * window_scale * GBE_WINDOW_W * window_scale * 3, sizeof(uint8_t));
  }
}

std::ostream &operator<<(std::ostream &out, const Window &win) {
  out.write(reinterpret_cast<const char*>(&win.state) , sizeof(win.state));
  return out;
}

std::istream &operator>>(std::istream &in, Window &win) {
  in.read(reinterpret_cast<char*>(&win.state) , sizeof(win.state));
  return in;
}
