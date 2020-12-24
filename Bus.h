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
	uint8_t ram[0xffff];
	uint8_t ppu_buffer;
	uint16_t temporary_register;
	uint16_t vram_addr;
	uint8_t vram_inc;
	int master_cycles = 0;
	bool address_latch = true;
	bool nmi_output = false;
	bool nmi_occured = false;
	bool vblank = false;
public:
	Bus();
	void ConnectBus(cpu6502*, NesPPU*);
	uint16_t BusRead(uint16_t);
	void BusWrite(uint16_t, uint8_t);
	void set_nmi() { nmi_occured = true; }
	void clear_nmi() { nmi_occured = false; }
	void set_vblank() { vblank = true; }
	void clr_vblank() { vblank = true; }
	void init();
	void run();
};

#endif // ! BUS_H


