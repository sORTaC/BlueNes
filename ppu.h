#ifndef NESPPU_H
#define NESPPU_H

#include <iostream>
//#include "Bus.h"

class Bus;

class NesPPU
{
private:
	uint8_t ppu_ram[0x4000];
	int ppu_cycles, ppu_scanline;
	uint8_t nametable;
	uint8_t vram_inc;
	uint8_t sprite_tb;
	uint8_t background;
	uint8_t generate_nmi;
	Bus* busPtr;
	uint8_t *ptrToBus;
public:
	void nametable_Set(int n) { nametable = n; }
	void vram_Set(int n) { vram_inc = n; }
	void spritetable_Set(int n) { sprite_tb = n; }
	void bkg_Set(int n) { background = n; }
	void gen_nmi_Set(int n) { generate_nmi = n; }
	void ppu_powerup();
	void ConnectToBus(Bus* ptr) { busPtr = ptr; }
	void ConnectPPU(uint8_t* ptr) { ptrToBus = ptr; }
	uint16_t ppu_read(uint16_t addr) { return ppu_ram[addr]; }
	void ppu_write(uint16_t addr, uint8_t data) { ppu_ram[addr] = data; }
	void setScanline(int n) { ppu_scanline = n; }
	void ppu_set_vblank();
	void ppu_clear_vblank();
	uint16_t interleave(uint16_t, uint16_t);
	void drawNametables();
	void step_ppu();
};

#endif //NESPPU_H



