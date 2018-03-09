#pragma once

#include <inttypes.h>
#include <functional>

class Memory;

class SerialPortInterface {
public:
	SerialPortInterface(Memory& MemRef, std::function<void(uint8_t)> on_byte_send = [](uint8_t){} ) :
          transfer_callback(on_byte_send), MEM(MemRef), transfer_bit(0), clock(0) {}

	void update(unsigned tclocks);

private:

	Memory& MEM;

  std::function<void(uint8_t)> transfer_callback;

	uint8_t transfer_bit;
	unsigned clock;

	void transfer();

	void finish();

};