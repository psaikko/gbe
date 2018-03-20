#include "gbe.h"

#include "buttons.h"
#include "sound.h"
#include "openal_output.h"
#include "cart.h"
#include "reg.h"
#include "timer.h"
#include "cpu.h"
#include "serial.h"
#include "gpu.h"
#include "mem.h"

gbe::gbe(std::string romfile) : clock_overflow(0) {

  BTN = new Buttons();
  SND = new Sound();
  REG = new Registers();
  CART = new Cart(romfile);
  MEM = new Memory(*CART, *BTN, *SND);
  GPU = new Gpu(*MEM);
  TIMER = new Timer(*MEM);
  CPU = new Cpu(*MEM, *REG);
  SERIAL = new SerialPortInterface(*MEM);

  REG->AF = 0x01B0;
  REG->BC = 0x0013;
  REG->DE = 0x00D8;
  REG->HL = 0x014D;
  REG->SP = 0xFFFE;
  REG->PC = 0x0100;

  *MEM->BIOS_OFF = 1;

  // enable LCD
  *MEM->LCD_CTRL = 0x80;
}

uint8_t* gbe::display() {
  return GPU->lcd_buffer.data();
}

void gbe::run(long clock_cycles) {

  clock_cycles += clock_overflow;

  while (clock_cycles > 0) {

    uint8_t opcode = MEM->readByte(REG->PC);
    Cpu::Instruction instr = CPU->instructions[opcode];

    if (!REG->HALT) {
      REG->PC += 1;
      instr.fn(*CPU);
    } else {
      REG->TCLK = 4;
    }

    GPU->update(REG->TCLK);
    TIMER->update(REG->TCLK);
    SERIAL->update(REG->TCLK);

    clock_cycles -= REG->TCLK;

    REG->TCLK = 0;
    CPU->handle_interrupts();

    GPU->update(REG->TCLK);
    TIMER->update(REG->TCLK);
    SERIAL->update(REG->TCLK);

    clock_cycles -= REG->TCLK;
  }

  clock_overflow = clock_cycles;
}

void gbe::run_to_vblank() {

  unsigned clock_cycles = 0;

  while (true) {

    uint8_t opcode = MEM->readByte(REG->PC);
    Cpu::Instruction instr = CPU->instructions[opcode];

    if (!REG->HALT) {
      REG->PC += 1;
      instr.fn(*CPU);
    } else {
      REG->TCLK = 4;
    }

    bool was_vblank = (*MEM->LCD_STAT & MODE_MASK) != MODE_VBLANK;

    GPU->update(REG->TCLK);
    TIMER->update(REG->TCLK);
    SERIAL->update(REG->TCLK);

    REG->TCLK = 0;
    CPU->handle_interrupts();

    GPU->update(REG->TCLK);
    TIMER->update(REG->TCLK);
    SERIAL->update(REG->TCLK);

    bool is_vblank = (*MEM->LCD_STAT & MODE_MASK) != MODE_VBLANK;

    // run until vblank triggered
    if (!was_vblank && is_vblank) break;
  }
}

void gbe::input(bool up, bool down, bool left, bool right, bool a, bool b, bool start, bool select) {
  BTN->state = 0;

  if (up) BTN->state |= KEY_UP;
  if (down) BTN->state |= KEY_DOWN;
  if (left) BTN->state |= KEY_LEFT;
  if (right) BTN->state |= KEY_RIGHT;
  if (start) BTN->state |= KEY_START;
  if (select) BTN->state |= KEY_SELECT;
  if (a) BTN->state |= KEY_A;
  if (b) BTN->state |= KEY_B;
}

uint8_t gbe::mem(uint16_t addr) {
  return MEM->readByte(addr);
}
