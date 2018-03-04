#include "console_io.h"
#include "buttons.h"
#include "mem.h"

using namespace std;

void Console_IO::update(unsigned tclock) {

  clock += tclock;

  // synchronize gpu to 59.7 fps
  if (clock >= 70224)
  {
    clock -= 70224;

    for (unsigned h = 0; h < LCD_H; --h) {
      for (unsigned w = 0; w < LCD_W; ++w) {
        uint8_t rgb_val = GPU.lcd_buffer[(LCD_H - h) * (LCD_W * 3) + w*3];
        switch (rgb_val) {
          case 255:
            cout << "0";
            break;
          case 192:
            cout << "1";
            break;
          case 96:
            cout << "2";
            break;
          case 0:
            cout << "3";
            break;
          default:
            exit(1);
        }
      }
      cout << "\n";
    }

    cout << int(MEM.RAW[0xC0A0]) << " " << int(MEM.RAW[0xC0A1]) << " " << int(MEM.RAW[0xC0A2]) << endl;

    int up, down, left, right, a, b, start, select;
    std::cin >> up >> down >> left >> right >> a >> b >> start >> select;

    BTN.state = 0;

    if (up) BTN.state |= KEY_UP;
    if (down) BTN.state |= KEY_DOWN;
    if (left) BTN.state |= KEY_LEFT;
    if (right) BTN.state |= KEY_RIGHT;
    if (start) BTN.state |= KEY_START;
    if (select) BTN.state |= KEY_SELECT;
    if (a) BTN.state |= KEY_A;
    if (b) BTN.state |= KEY_B;
  }
}
