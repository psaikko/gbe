#pragma once

#include "UI.h"

class Headless : public UI {
 public:
  Headless() : UI() {};

  void update(unsigned tclock) {};

  void read(std::istream & in) {};
  void write(std::ostream & in) const {};
};
