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
	bool horizontal_mirroring = true;
	Bus* busPtr;
	uint8_t *ptrToBus;
public:
	void ppu_powerup();
	void ppu_set_vblank();
	void ppu_clear_vblank();
	void ConnectToBus(Bus* ptr) { busPtr = ptr; }
	void ConnectPPU(uint8_t* ptr) { ptrToBus = ptr; }
	uint16_t ppu_read(int addr);
	void ppu_write(int addr, uint8_t data);
	void setScanline(int n) { ppu_scanline = n; }
	uint16_t interleave(uint16_t, uint16_t);
	void drawNametable();
	void step_ppu();
};

#endif //NESPPU_H



