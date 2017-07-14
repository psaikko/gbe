# gbe : Game Boy Emulator

Work in progress .. most games correctly emulated
````
install libalut-dev libopenal-dev libglew-dev libglm-dev
make
./gbe [-B path_to_bios] -R path_to_rom
````

Features
---
gbe displays the main game window, tilesets, and tilemaps.

Esc closes the emulator.

Shift+[F1-F4] disable individual sound channels.

[F1-F4] enable individual sound channels.

Screenshots
---
Game view:

![Game window](https://raw.githubusercontent.com/psaikko/gbe/master/img/Game_screenshot.png)

Tileset view:

![Tileset window](https://raw.githubusercontent.com/psaikko/gbe/master/img/Tileset_screenshot.png)

TODOs
---
- Savestates
- Usable UI
- Display scaling
- Noise channel accuracy
- Sprite display limits
- Intra-scanline timing
- Joypad interrupts
- Some memory bank controllers
- ... and more
