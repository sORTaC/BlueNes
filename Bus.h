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
	uint8_t ppu_buffer;
	uint16_t temporary_register;
	uint16_t vram_addr;
	uint16_t oam_addr;
	uint8_t vram_inc;
	uint16_t dma_page;
	int background_table;
	int sprite_table;
	int size;
	bool address_latch = true;
	bool nmi_output = false;
	bool nmi_occured = false;
	bool vblank = false;
public:
	Bus();
	void ConnectBus(cpu6502*, NesPPU*);
	uint16_t BusRead(uint16_t);
	void BusWrite(uint16_t, uint8_t);
	void set_nmi(bool m) { nmi_occured = m; }
	void set_vblank(bool m) { vblank = m; }
	int return_bkg() { return background_table; }
	int return_spr() { return sprite_table; }
	int return_size() { return size; }
	void init();
	void run();
};

#endif // ! BUS_H


