#pragma once

#include "UI.h"

class Headless : public UI {
  public:
    Headless() : UI(){};

    void update(unsigned){};

    void read(std::istream &){};
    void write(std::ostream &) const {};
};
