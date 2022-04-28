#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "buttons.h"
#include "mem.h"
#include "sound.h"

uint8_t *Memory::getReadPtr(uint16_t addr) {
    // switch by 8192 byte segments
    switch (addr >> 12) {
        case 0x0:
            if ((!*BIOS_OFF) && addr < 0x0100) {
                return &BIOS[addr];
            }
            [[fallthrough]];
        case 0x1:
        case 0x2:
        case 0x3:
            // return &ROM0[addr]; // ROM0
            return CART.rom0Ptr(addr);
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            // return &ROM1[addr - 0x4000]; // ROM1
            return CART.rom1Ptr(addr - 0x4000);
        case 0x8:
        case 0x9:
            return &RAW[addr]; // grRAM
        case 0xA:
        case 0xB:
            // extRAM or RTC
            return CART.ramPtr(addr - 0xA000);
        case 0xC:
        case 0xD:
            return &RAW[addr];   // RAM
        default:                 // E, F
            if (addr < 0xFE00) { // shadow RAM
                return &RAW[addr & 0xDFFF];
            } else {
                switch (addr & 0xFF80) {
                    case 0xFE00: // SPR
                    case 0xFE80:
                        if (addr < 0xFEA0)
                            return &RAW[addr];
                        else
                            return nullptr;
                    case 0xFF00: // IO
                        return &RAW[addr];
                    case 0xFF80: // ZERO
                        return &RAW[addr];
                    default:
                        assert(false);
                }
            }
    }
}

uint8_t *Memory::getWritePtr(uint16_t addr) {
    // switch by 8192 byte segments
    switch (addr >> 12) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            // ROM0, ROM1
            return nullptr;
        case 0x8:
        case 0x9:
            // grRAM
            return &RAW[addr];
        case 0xA:
        case 0xB:
            // extRAM or RTC
            return CART.ramPtr(addr - 0xA000);
        case 0xC:
        case 0xD:
            // RAM
            return &RAW[addr];
        default:                 // E, F
            if (addr < 0xFE00) { // shadow RAM
                return &RAW[addr & 0xDFFF];
            } else {
                switch (addr & 0xFF80) {
                    case 0xFE00: // SPR
                    case 0xFE80:
                        if (addr < 0xFEA0)
                            return &RAW[addr];
                        else
                            return nullptr;
                    case 0xFF00: // IO
                        if (addr == 0xFF50 && *BIOS_OFF) {
                            return nullptr;
                        }
                        if (addr == 0xFF44) {
                            return nullptr;
                        }
                        return &RAW[addr];
                    case 0xFF80: // ZERO
                        return &RAW[addr];
                    default:
                        assert(false);
                }
            }
    }
}

uint8_t Memory::readByte(uint16_t addr) {
    if (break_addr == addr)
        at_breakpoint = true;

    if (addr >= 0xFF10 && addr <= 0xFF3F) {
        return SND.readByte(addr);
    }

    uint8_t *ptr = getReadPtr(addr);

    if (addr == 0xFF00) { // JOYPAD
        return ~(*ptr);
    }

    if (ptr != nullptr)
        return *ptr;
    else {
        fprintf(stdout, "[Warning] Attempting read from address 0x%04X\n", addr);
        return 0;
    }
}

uint16_t Memory::readWord(uint16_t addr) {
    assert(!(addr >= 0xFF10 && addr <= 0xFF26)); // not a sound register
    if (break_addr == addr)
        at_breakpoint = true;
    uint8_t *ptr = getReadPtr(addr);
    if (ptr != nullptr)
        return *reinterpret_cast<uint16_t *>(ptr);
    else {
        fprintf(stdout, "[Warning] Attempting read from address 0x%04X\n", addr);
        return 0;
    }
}

void Memory::writeByte(uint16_t addr, uint8_t val) {
    if (break_addr == addr)
        at_breakpoint = true;

    if (addr == 0xFF41) {
        *LCD_STAT = (val & ~7) || (*LCD_STAT & 7);
        return;
    }

    if (addr >= 0xFF10 && addr <= 0xFF3F) {
        SND.writeByte(addr, val);
        return;
    }

    uint8_t *ptr = getWritePtr(addr);

    // if (addr >= 0xFF04 && addr <= 0xFF07) {
    //   printf("[timer] 0x%04X write 0x%02X\n", addr, val);
    // }

    if (addr == 0xFF00) { // JOYPAD
        // printf("[joypad] write (0x%02X)\n", val);
        *ptr &= 0x0F;
        if (val == 0x10) {
            *ptr |= 0x20;
            *ptr &= 0xF0;
            // Load A, B, Select, Start bits
            *ptr |= BTN.key_state();
        } else if (val == 0x20) {
            *ptr |= 0x10;
            *ptr &= 0xF0;
            // Load Right, Left, Up, Down bits
            *ptr |= BTN.dpad_state();
        } else if (val == 0x30) {
            // TODO: should we reset lower 4 bits here?
            *ptr &= 0xF0;
        } else {
            printf("[Warning] Bad write (0x%02X) to JOYP (0x%04X)\n", val, addr);
        }

        return;
    } else if (ptr == OAM_DMA) {
        // TODO: block memory access
        // printf("OAM DMA\n");
        // assert(val <= 0xF1);
        for (uint8_t low = 0x00; low <= 0xF9; ++low) {
            RAW[0xFE00 + low] = readByte((((uint16_t)val) << 8) + low);
        }
    } else if (addr == 0xFF04) {
        // divider register reset on write
        *ptr = 0;
        return;
    }

    switch (CART.bank_controller) {
        case Cart::mbc_type::NONE:
            break;
        case Cart::mbc_type::MBC1:
            if (addr <= 0x1FFF) {
                // if (val & 0x0A) {
                //     printf("[cart] ram enable\n");
                // } else {
                //     printf("[cart] ram disable\n");
                // }
                // printf("RAM enable / disable 0x%02X at 0x%04X\n", val, addr);
                return;
            } else if (0x2000 <= addr && addr <= 0x3FFF) {
                // printf("ROM bank selection 0x%02X at 0x%04X\n", val, addr);
                val &= 0x1F;
                if (val == 0)
                    val = 1;
                // ROM1 = CART.romBank(val);
                CART.rom_bank = val;
                return;
            } else if (0x4000 <= addr && addr <= 0x5FFF) {
                // printf("RAM bank selection 0x%02X at 0x%04X\n", val, addr);
                val &= 0x03;
                if (CART.mbc_mode == Cart::controller_mode::RAM_banking) {
                    assert(val <= 4);
                    CART.ram_bank = val;
                } else {
                    // ROM1 = CART.romBank((CART.rom_bank & 0x1F) | (val << 5));
                    CART.rom_bank = (CART.rom_bank & 0x1F) | (val << 5);
                }
                return;
            } else if (0x6000 <= addr && addr <= 0x7FFF) {
                if (val == 1) {
                    CART.mbc_mode = Cart::controller_mode::RAM_banking;
                } else {
                    CART.mbc_mode = Cart::controller_mode::ROM_banking;
                }
            }
            break;
        case Cart::mbc_type::MBC3:
            if (addr <= 0x1FFF) {
                // printf("RAM enable / disable 0x%02X at 0x%04X\n", val, addr);
                return;
            }
            if (0x2000 <= addr && addr <= 0x3FFF) {
                // printf("ROM bank selection 0x%02X at 0x%04X\n", val, addr);
                val &= 0x7F;
                if (val == 0)
                    val = 1;
                // ROM1 = CART.romBank(val);
                CART.rom_bank = val;
                return;
            }
            if (0x4000 <= addr && addr <= 0x5FFF) {
                // printf("RAM bank selection 0x%02X at 0x%04X\n", val, addr)
                val &= 0x1F;
                if (val <= 0x04) {
                    CART.ram_bank   = val;
                    CART.RTC_access = false;
                } else if (val >= 0x08 && val <= 0x0C) {
                    // RTC register select
                    CART.RTC_reg_select = unsigned(val - 0x08);
                    CART.RTC_access     = true;
                } else {
                    assert(false);
                }
                return;
            }
            if (0x6000 <= addr && addr <= 0x7FFF) {
                // printf("[cart] RTC latch %d\n", val);
                return;
            }
            break;
        default:
            printf("Unimplemented memory bank controller type.");
            exit(1);
    }

    if (ptr == nullptr) {
        fprintf(stdout, "[Warning] Attempting write to address 0x%04X\n", addr);
        return;
    }
    *ptr = val;
}

void Memory::writeWord(uint16_t addr, uint16_t val) {
    assert(!(addr >= 0xFF10 && addr <= 0xFF26));
    if (break_addr == addr)
        at_breakpoint = true;
    uint8_t *ptr = getWritePtr(addr);

    if (ptr == nullptr) {
        fprintf(stdout, "[Warning] Attempting write to address 0x%04X\n", addr);
        return;
    }
    uint16_t *wptr = reinterpret_cast<uint16_t *>(ptr);
    *wptr          = val;
}

uint64_t Memory::checksum() const {
    uint64_t sum = 0;

    for (uint8_t ch : RAW)
        sum += ch;

    return sum;
}

ostream &operator<<(ostream &out, const Memory &mem) {
    cout << "Write " << mem.checksum() << endl;
    out.write(reinterpret_cast<const char *>(mem.RAW), sizeof(mem.RAW));
    return out;
}

istream &operator>>(istream &in, Memory &mem) {
    cout << "State " << mem.checksum() << endl;
    in.read(reinterpret_cast<char *>(mem.RAW), sizeof(mem.RAW));
    cout << "Read " << mem.checksum() << endl;
    return in;
}