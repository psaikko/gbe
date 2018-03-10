# gbe : Game Boy Emulator

Work in progress .. most games correctly emulated
````
install libalut-dev libglfw3-dev libglm-dev libglew-dev
cmake . && make
./gbe [-B path_to_bios] -R path_to_rom
````

Features
---
gbe displays the main game window, tilesets, and tilemaps.

Esc closes the emulator.

Shift+[F1-F4] disable individual sound channels.

[F1-F4] enable individual sound channels.

[F5] Saves state.

[F6] Loads state.

Screenshots
---
Game view:

![Game window](https://raw.githubusercontent.com/psaikko/gbe/master/img/Game_screenshot.png)

Tileset view:

![Tileset window](https://raw.githubusercontent.com/psaikko/gbe/master/img/Tileset_screenshot.png)

Python wrapper
---
To install
1. `git clone --recursive https://github.com/psaikko/gbe/` 
2. `cmake . && make libgbe`
3. `cp {gbe.h,libgbe.a} py-wrapper/`
4. `pip install py-wrapper/`

Usage

```
import libgbe
gbe = GBE("path/to/rom")
gbe.run(70224)
gbe.display()
```

TODOs
---
- Cartridge saves
- Multiple savestates
- Usable UI
- GPU display scaling
- Sprite display limits
- Intra-scanline timing
- Joypad interrupts
- Serial I/O
- Some memory bank controllers
- ... and more
