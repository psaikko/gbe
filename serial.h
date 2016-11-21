#pragma once

#include <inttypes.h>

class Memory;

class SerialPortInterface {
public:
	SerialPortInterface(Memory& MemRef) : MEM(MemRef), transfer_bit(0), clock(0) {} 

	void update(unsigned tclocks);

private:

	Memory& MEM;

	uint8_t transfer_bit;
	unsigned clock;

	void transfer();

	void finish();

};