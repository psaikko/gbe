#pragma once

#include <cstring>
#include <inttypes.h>

#include "cart.h"

typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile_id;
    struct {
        union {
            uint8_t flags;
            struct {
                uint8_t _ : 4;
                uint8_t palette : 1;
                uint8_t xflip : 1;
                uint8_t yflip : 1;
                uint8_t priority : 1;
            };
        };
    };
} oam_entry;

class Buttons;
class Sound;

class Memory {

  public:
    Memory(Cart &CartRef, Buttons &BtnRef, Sound &SndRef) : BTN(BtnRef), CART(CartRef), SND(SndRef) {

        // Don't read ROM or cart RAM from RAW
        // Fill with sentinel value (unused instruction 0xDD)
        memset(&RAW[0x0000], 0xDD, 0x4000);
        memset(&RAW[0x4000], 0xDD, 0x4000);
        memset(&RAW[0xA000], 0xDD, 0x2000);
    }

    Buttons &BTN;
    Cart &CART;
    Sound &SND;

    uint8_t RAW[65536]; // TODO

    uint8_t *const RAM   = &RAW[0xC000];
    uint8_t *const _RAM  = &RAW[0xE000];
    oam_entry *const OAM = (oam_entry *)(&RAW[0xFE00]);
    uint8_t *const IO    = &RAW[0xFF00];
    uint8_t *const ZERO  = &RAW[0xFF80];

    uint8_t BIOS[256];

    uint8_t *const SB       = &RAW[0xFF01];
    uint8_t *const SC       = &RAW[0xFF02];
    uint8_t *const DIV      = &RAW[0xFF04];
    uint8_t *const TIMA     = &RAW[0xFF05];
    uint8_t *const TMA      = &RAW[0xFF06];
    uint8_t *const TAC      = &RAW[0xFF07];
    uint8_t *const IF       = &RAW[0xFF0F];
    uint8_t *const LCD_CTRL = &RAW[0xFF40];
    uint8_t *const LCD_STAT = &RAW[0xFF41];
    uint8_t *const SCRL_Y   = &RAW[0xFF42];
    uint8_t *const SCRL_X   = &RAW[0xFF43];
    uint8_t *const SCAN_LN  = &RAW[0xFF44]; // TODO: readonly
    uint8_t *const LN_CMP   = &RAW[0xFF45];
    uint8_t *const OAM_DMA  = &RAW[0xFF46]; // TODO: writeonly
    uint8_t *const BG_PLT   = &RAW[0xFF47]; // TODO: writeonly
    uint8_t *const OBJ0_PLT = &RAW[0xFF48]; // TODO: writeonly
    uint8_t *const OBJ1_PLT = &RAW[0xFF49]; // TODO: writeonly
    uint8_t *const WIN_Y    = &RAW[0xFF4A];
    uint8_t *const WIN_X    = &RAW[0xFF4B];
    uint8_t *const BIOS_OFF = &RAW[0xFF50];
    uint8_t *const IE       = &RAW[0xFFFF];

    uint8_t *const TILESET1 = &RAW[0x8000];
    uint8_t *const TILESET0 = &RAW[0x8800];
    uint8_t *const TILEMAP0 = &RAW[0x9800];
    uint8_t *const TILEMAP1 = &RAW[0x9C00];

    uint16_t break_addr = 0;
    bool at_breakpoint  = false;

    uint8_t *getReadPtr(uint16_t addr);

    uint8_t *getWritePtr(uint16_t addr);

    uint8_t readByte(uint16_t addr);

    uint16_t readWord(uint16_t addr);

    void writeByte(uint16_t addr, uint8_t val);

    void writeWord(uint16_t addr, uint16_t val);

    uint64_t checksum() const;

    friend std::ostream &operator<<(std::ostream &out, const Memory &mem);
    friend std::istream &operator>>(std::istream &in, Memory &mem);
};
