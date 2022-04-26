# gbe : Game Boy Emulator

Work in progress .. most games correctly emulated

## Build on Ubuntu
Tested on 20.04 LTS, 22.04 LTS, WSL2
````
sudo apt install g++ cmake
sudo apt install libalut-dev libglfw3-dev libglm-dev libglew-dev
cmake -DCMAKE_BUILD_TYPE=Release . && make gbe
./gbe [-B path_to_bios] -R path_to_rom
````

## Build on Windows
Tested on Windows 11 with msvc 19.31.31107.

Install standalone [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/?q=build+tools#build-tools-for-visual-studio-2022).

Add locations of build tools cmake and msbuild to `Path` environment variable:
````
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe
````

Install dependencies
````
git submodule update --init vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg.exe install glfw3 glew glm openal-soft freealut getopt --triplet=x64-windows
````

Build and run:
````
cmake.exe -DCMAKE_CONFIGURATION_TYPES=Release -S. -B.\build
MSBuild.exe .\build\gbe.sln
.\build\Release\gbe.exe [-B path_to_bios] -R path_to_rom
````

## Features
---
gbe displays the main game window, tilesets, and tilemaps.

Esc closes the emulator.

Shift+[F1-F4] disable individual sound channels.

[F1-F4] enable individual sound channels.

[F5] Saves state.

[F6] Loads state.


## Screenshots
---
Game view:

![Game window](https://raw.githubusercontent.com/psaikko/gbe/master/img/Game_screenshot.png)

Tileset view:

![Tileset window](https://raw.githubusercontent.com/psaikko/gbe/master/img/Tileset_screenshot.png)

## Python wrapper
---
To install

0. `sudo apt install pybind11-dev python3-dev`
1. `pip install pybind11`
2. `pip install ./`

Usage

```
from libgbe import GBE
gbe = GBE("path/to/rom")
gbe.run(70224)
gbe.display()
```

## TODOs
---
- Cartridge saves
- Cartridge realtime clock
- Multiple savestates
- Usable UI
- GPU display scaling shader
- Intra-scanline timing
- Joypad interrupts
- Serial I/O
- Some memory bank controllers
- ... and more
