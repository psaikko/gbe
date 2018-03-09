#pragma once

#include <iostream>

class UI {

 public:

  UI() : breakpoint(false), save_state(false), load_state(false), close(false) {}

  virtual void update(unsigned tclock) = 0;

  bool breakpoint;
  bool save_state;
  bool load_state;
  bool close;

  virtual void read(std::istream & in) = 0;
  virtual void write(std::ostream & out) const = 0;

  friend std::ostream & operator << (std::ostream & in, const UI & ui) {
    ui.write(in);
    return in;
  }

  friend std::istream & operator >> (std::istream & out, UI & ui) {
    ui.read(out);
    return out;
  }

};

