#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include "gbe.h"

// runs test with output to serial
bool run_test_rom_serial(std::string rom_path) {
    std::stringstream serial_out_stream;
    gbe emu(rom_path, [&](uint8_t b) { serial_out_stream << (char)b; });
    while (emu.run_to_vblank());
    std::string output_str = serial_out_stream.str();
    std::cout << output_str << std::endl;

    return output_str.find("Passed") != std::string::npos;
}

// runs test with output to memory
bool run_test_rom_memory(std::string rom_path) {
    gbe emu(rom_path);
    while (emu.run_to_vblank());
    uint8_t status = emu.mem(0xA000);
    uint8_t chk_1 = emu.mem(0xA001);
    uint8_t chk_2 = emu.mem(0xA002);
    uint8_t chk_3 = emu.mem(0xA003);

    // Magic numbers indicate test could be run
    if (chk_1 == 0xDE && chk_2 == 0xB0 && chk_3 == 0x61) {
        std::cerr << "Signature OK" << std::endl;
        std::cerr << "Status " << std::hex << int(status) << std::endl;
        const int max_chars = 512;
        std::stringstream memory_out_stream;
        // On a successful test run, this rom outputs a zero-terminated string to 0xA004
        for (int i = 0; i < max_chars; i++) {
            uint8_t byte = emu.mem(0xA004 + i);
            if (byte == 0) {
                break;
            }
            memory_out_stream << byte;
        }
        std::cout << memory_out_stream.str() << std::endl;
        return status == 0;
    } else {
        std::cerr << "Signature BAD: "
                  << std::setfill('0')
                  << std::uppercase
                  << std::hex
                  << std::setw(2) << int(chk_1) << " "
                  << std::setw(2) << int(chk_2) << " "
                  << std::setw(2) << int(chk_3)
                  << std::endl;
        return false;
    }

    return false;
}

int main(int argc, char **argv) {
    std::vector<std::string> argList(argv, argv + argc);
    bool ok;

    if (argList[1] == "serial") {
        ok = run_test_rom_serial(argList[2]);
    } else if (argList[1] == "memory") {
        ok = run_test_rom_memory(argList[2]);
    }

    return (ok ? 0 : 1);
}
