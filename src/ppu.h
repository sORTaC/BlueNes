#ifndef NESPPU_H
#define NESPPU_H

#include "cpu.h"
#include "mem.h"

#define ppuctrl 0x2000
#define ppumask 0x2001
#define ppustatus 0x2002
#define oamddr 0x2003
#define oamdata 0x2004
#define ppuscroll 0x2005
#define ppuaddr 0x2006
#define ppudata 0x2007
#define oammdma 0x4014

class NesPPU
{
private:
	mem6502 ppumem;
	cpu6502* cpu;
	bool horizontal = 0;
	bool vertical = 0;
	bool nmi_occured = false;
	bool nmi_output = false;
	bool address_latch = false;
	uint8_t ppu_buffer;
	uint8_t previously_written_to_register;
	uint16_t temporary_register;
	uint16_t effective_register;
	uint16_t vram_addr;
public:
	NesPPU(mem6502 _mem, cpu6502* cpu);

	//ppu write and read to ppu ram
	void ppuwrite(uint16_t addr, uint8_t data);
	uint16_t ppuread(uint16_t addr);

	//ppu general functions
	void power_up();
	void ppu_reset();

	//ppu flag functions
	void ppu_set_horizontal() { horizontal = 1; }
	void ppu_set_vertical() { vertical = 1; }

	//ppu render
	uint16_t interleave(uint16_t, uint16_t);
	void Render(int);
	uint16_t FetchNT(int x, int y);
	uint16_t FetchAT(int x, int y);
};

#endif //NESPPU_H



