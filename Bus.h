#ifndef  BUS_H
#define  BUS_H

#include <vector>
#include "cpu.h"
#include "ppu.h"

class Bus
{
private:
	cpu6502* cpu;
	NesPPU* ppu;
	uint8_t ram[65536];
	uint8_t* keys;
public:
	Bus();
	void ConnectBus(cpu6502*, NesPPU*);
	uint16_t BusRead(uint16_t);
	void BusWrite(uint16_t, uint8_t);
	uint8_t ask_cpu(uint16_t data);
	void set_controller(uint8_t*);
	uint8_t read_controller(uint8_t select);
	void init();
	void run();
};

#endif // ! BUS_H


