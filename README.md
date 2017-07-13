# gbe : Game Boy Emulator

Work in progress .. most games correctly emulated
````
install libalut-dev libopenal-dev libglew-dev libglm-dev
make
./gbe [-B path_to_bios] -R path_to_rom
````
gbe displays the main game window, tilesets, and tilemaps.

Esc closes the emulator.
Shift+[F1-F4] disables sound channels.
[F1-F4] enable sound channels.


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
