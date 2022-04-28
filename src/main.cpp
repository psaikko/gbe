#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>

#include "buttons.h"
#include "cart.h"
#include "cpu.h"
#include "gbe.h"
#include "gpu.h"
#include "mem.h"
#include "openal_output.h"
#include "reg.h"
#include "serial.h"
#include "sound.h"
#include "sync.h"
#include "timer.h"

#include "headless.h"
#include "window.h"

using namespace std;

void printRegisters(Memory &MEM, Registers &REG, bool words) {
    if (!words) {
        printf(
            "A %02X F %02X B %02X C %02X D %02X E %02X H %02X L %02X\n", REG.A, REG.F, REG.B, REG.C, REG.D, REG.E,
            REG.H, REG.L
        );
    } else {
        printf(
            "AF %04X BC %04X DE %04X HL %04X SP %04X PC %04X (LY) %02X\n", REG.AF, REG.BC, REG.DE, REG.HL, REG.SP,
            REG.PC, MEM.readByte(0xFF44)
        );
    }
}

void printInstruction(Memory &MEM, Registers &REG, Cpu &CPU) {
    uint8_t opcode         = MEM.readByte(REG.PC);
    Cpu::Instruction instr = CPU.instructions[opcode];
    printf("Instruction 0x%02X at 0x%04X: ", opcode, REG.PC);
    if (instr.argw == 0)
        printf("%s", instr.name);
    else if (instr.argw == 1)
        printf(instr.name, MEM.readByte(REG.PC + 1));
    else if (instr.argw == 2)
        printf(instr.name, MEM.readWord(REG.PC + 1));
    printf("\n");

    if (opcode == 0xCB) {
        uint8_t ext_opcode = MEM.readByte(REG.PC + 1);
        printf("        Ext 0x%02X at 0x%04X: ", ext_opcode, REG.PC + 1);
        printf("%s", CPU.ext_instructions[MEM.readByte(REG.PC + 1)].name);
        printf("\n");
    }
}

void readBIOSFile(Memory &MEM, const string filename) {
    ifstream biosfile(filename, ios::binary);
    biosfile.read((char *)MEM.BIOS, 256);
    biosfile.close();
}

int main(int argc, char **argv) {

    int log_register_bytes = false, log_register_words = false, log_flags = false, log_gpu = false,
        log_instructions = false, breakpoint = false, mem_breakpoint = false, stepping = false, load_bios = false,
        load_rom = false, unlocked_frame_rate = false, log_serial = false, headless = false;

    string romfile, biosfile;

    uint16_t breakpoint_addr     = 0;
    uint16_t mem_breakpoint_addr = 0;

    unsigned long long instruction_limit = 0;

    int c;

    while (true) {
        static struct option long_options[] {
            {"rw", no_argument, &log_register_words, 1}, {"rb", no_argument, &log_register_bytes, 1},
                {"gpu", no_argument, &log_gpu, 1}, {"headless", no_argument, nullptr, 'h'},
                {"log-serial", no_argument, nullptr, 'S'}, {"instruction-limit", required_argument, nullptr, 'L'},
                {"instructions", no_argument, nullptr, 'i'}, {"flags", no_argument, nullptr, 'f'},
                {"unlockfps", no_argument, nullptr, 'u'}, {"console", no_argument, nullptr, 'c'},
                {"bios", required_argument, nullptr, 'B'}, {"rom", required_argument, nullptr, 'R'},
                {"breakpoint", required_argument, nullptr, 'b'}, {"step", required_argument, nullptr, 's'},
                {"memory-breakpoint", required_argument, nullptr, 'M'}, {
                nullptr, 0, nullptr, 0
            }
        };

        int option_index = 0;
        c                = getopt_long(argc, argv, "s:b:ifucB:R:M:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c) {
            case 0:
                // If this option set a flag, do nothing else now.
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'B':
                load_bios = true;
                biosfile  = string(optarg);
                break;

            case 'R':
                load_rom = true;
                romfile  = string(optarg);
                break;

            case 'b':
                breakpoint = true;
                printf("option -b with value `%s'\n", optarg);
                breakpoint_addr = std::stoi(optarg, 0, 0);
                break;

            case 'M':
                printf("option -M with value `%s'\n", optarg);
                mem_breakpoint      = true;
                mem_breakpoint_addr = std::stoi(optarg, 0, 0);
                break;

            case 'f':
                log_flags = true;
                break;

            case 'i':
                log_instructions = true;
                break;

            case 'u':
                unlocked_frame_rate = true;
                break;

            case 'S':
                log_serial = true;
                break;

            case 'L':
                instruction_limit = stoull(optarg, 0, 0);
                break;

            case 'h':
                headless = true;
                break;

            case '?':
                // getopt_long already printed an error message.
                break;

            default:
                abort();
        }
    }

    if (optind < argc) {
        fprintf(stderr, "[warning]: Unhandled args");
        while (optind < argc)
            fprintf(stderr, " %s", argv[optind++]);
        fprintf(stderr, "\n");
    }

    if (!load_rom) {
        printf("load ROM with -R <file>\n");
        exit(0);
    }

    Buttons BTN;
    Sound SND;
    OpenAL_Output SND_OUT(SND);
    Cart CART(romfile);
    Memory MEM(CART, BTN, SND);

    Registers REG;

    Gpu GPU(MEM);

    UI *interface = headless ? static_cast<UI *>(new Headless())
                             : static_cast<UI *>(new Window(MEM, BTN, SND, GPU, unlocked_frame_rate));

    Timer TIMER(MEM);

    Cpu CPU(MEM, REG);

    std::function<void(uint8_t)> serial_callback = [](uint8_t) {};
    if (log_serial) {
        serial_callback = [](uint8_t b) { printf("[serial] %d\n", b); };
    }
    SerialPortInterface SERIAL(MEM, serial_callback);

    if (load_bios) {
        readBIOSFile(MEM, biosfile);
    } else {
        REG.AF            = 0x01B0;
        REG.BC            = 0x0013;
        REG.DE            = 0x00D8;
        REG.HL            = 0x014D;
        REG.SP            = 0xFFFE;
        REG.PC            = 0x0100;
        *MEM.BIOS_OFF     = 1;
        *MEM.LCD_CTRL     = 0x91;
        *MEM.LCD_STAT     = 0x05;
        *MEM.IF           = 0x03;
        *MEM.SCAN_LN      = 0;
        GPU.state.clk     = 408;
        GPU.state.enabled = true;
    }

    MEM.break_addr = mem_breakpoint_addr;

    // enable LCD
    *MEM.LCD_CTRL |= 0x80;

    // start audio/video sync timer
    SyncTimer::get().start();

    unsigned long long clk = 0;

    while (!interface->close) {

        bool is_breakpoint = (breakpoint && (REG.PC == breakpoint_addr)) || (mem_breakpoint && (MEM.at_breakpoint)) ||
                             interface->breakpoint;

        if (interface->save_state) {
            ofstream file("gbe.state", ifstream::binary);
            file << REG;
            file << MEM;
            file << SND_OUT;
            file << GPU;
            file << CART;
            file << *interface;
            file << SyncTimer::get();
            file.close();
        }

        if (interface->load_state) {
            ifstream file("gbe.state", ifstream::binary);
            file >> REG;
            file >> MEM;
            file >> SND_OUT;
            file >> GPU;
            file >> CART;
            file >> *interface;
            file >> SyncTimer::get();
            file.close();
        }

        interface->load_state = false;
        interface->save_state = false;
        interface->breakpoint = false;

        MEM.at_breakpoint = false;

        uint8_t opcode         = MEM.readByte(REG.PC);
        Cpu::Instruction instr = CPU.instructions[opcode];

        if (!REG.HALT) {
            if (log_register_bytes)
                printRegisters(MEM, REG, false);
            if (log_register_words)
                printRegisters(MEM, REG, true);
            if (log_flags) {
                printf("Z %1d N %1d H %1d C %1d\n", REG.FLAG_Z, REG.FLAG_N, REG.FLAG_H, REG.FLAG_C);
                printf(
                    "LCD_CTRL %02X LCD_STAT %02x SCAN_LN %02X PLT %02X BIOS_OFF %1X\n", *MEM.LCD_CTRL, *MEM.LCD_STAT,
                    *MEM.SCAN_LN, *MEM.BG_PLT, *MEM.BIOS_OFF
                );
                printf("IME %X IE %02X IF %02X ROM%d RAM%d\n", REG.IME, *MEM.IE, *MEM.IF, CART.rom_bank, CART.ram_bank);
            }
            if (log_gpu) {
                printf("GPU CLK: 0x%04X LINE: 0x%02X\n", GPU.state.clk, *MEM.SCAN_LN);
            }

            if (log_instructions)
                printInstruction(MEM, REG, CPU);
        }

        if (stepping || is_breakpoint) {

            stepping     = true;
            bool parsing = true;
            bool more    = false;
            printf("%04X> ", REG.PC);
            while (parsing) {
                switch (getchar()) {
                    case 'R':
                        // toggle register word logging
                        log_register_words = !log_register_words;
                        break;
                    case 'r':
                        // toggle register byte logging
                        log_register_bytes = !log_register_bytes;
                        break;
                    case 'i':
                        // toggle instruction logging
                        log_instructions = !log_instructions;
                        break;
                    case 'I':
                        // display current state
                        printRegisters(MEM, REG, true);
                        printInstruction(MEM, REG, CPU);
                        more = true;
                        break;
                    case 's':
                        // turn on stepping
                        stepping = true;
                        break;
                    case 'c':
                        // turn off stepping
                        stepping = false;
                        break;
                    case 'q':
                        // quit
                        exit(1);
                    case 'n':
                        // skip to instruction at PC+1
                        stepping        = false;
                        breakpoint      = true;
                        breakpoint_addr = REG.PC + (1 + instr.argw);
                        break;
                    case 'b':
                        // set breakpoint
                        if (!scanf("%hX", &breakpoint_addr))
                            break;
                        stepping   = false;
                        breakpoint = true;
                        break;
                    case 'M':
                        // set memory breakpoint
                        if (!scanf("%hX", &MEM.break_addr))
                            break;
                        stepping       = false;
                        mem_breakpoint = true;
                        break;
                    case 'd':
                        // dump memory range
                        uint16_t addr;
                        uint16_t len;
                        if (!scanf("%hX %hX", &addr, &len))
                            break;
                        for (uint16_t i = 0; i < len; ++i) {
                            printf("%02X ", MEM.readByte(addr + i));
                            if (((i + 1) % 8 == 0) || (i == len - 1))
                                printf("\n");
                        }
                        more = true;
                        break;
                    case 'f':
                        // toggle flag logging
                        log_flags = !log_flags;
                        break;
                    case '\n':
                        parsing = more;
                        more    = false;
                        if (parsing)
                            printf("%04X> ", REG.PC);
                        break;
                }
            }
        }

        if (!REG.HALT) {
            REG.PC += 1;
            instr.fn(CPU);
        } else {
            REG.TCLK = 4;
        }

        GPU.update(REG.TCLK);
        TIMER.update(REG.TCLK);
        SERIAL.update(REG.TCLK);
        SND.update(REG.TCLK);

        interface->update(REG.TCLK);

        clk += REG.TCLK;

        REG.TCLK = 0;
        CPU.handle_interrupts();

        if (REG.TCLK != 0) {
            GPU.update(REG.TCLK);
            TIMER.update(REG.TCLK);
            SERIAL.update(REG.TCLK);
            SND.update(REG.TCLK);
        }

        interface->update(REG.TCLK);

        SND_OUT.update_buffer();

        clk += REG.TCLK;
        if (instruction_limit && clk > instruction_limit)
            break;
        if (CPU.is_stuck()) {
            printf("[warning] infinite loop detected\n");
            break;
        }
    }

    printf("Time: %lld ms\n", SyncTimer::get().elapsed_ms());
    printf("Clk: %lld\n", clk);
    printf("Samples generated: %lu\n", SND.samples);
    printf("Samples played: %lu\n", SND_OUT.samples);
}