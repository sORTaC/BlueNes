#ifndef NESPPU_H
#define NESPPU_H

#include <iostream>
//#include "Bus.h"

class Bus;

class NesPPU
{
private:
	uint8_t ppu_ram[0x4000];
	uint8_t primary_oam[64 * 4];
	uint8_t secondary_oam[64];
	int ppu_cycles, ppu_scanline;
	bool horizontal_mirroring = false;
	Bus* busPtr;
	uint8_t *ptrToBus;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	Uint32* pixels;
public:
	NesPPU();
	~NesPPU();
	void ppu_powerup();
	void ppu_set_vblank();
	void ppu_clear_vblank();
	void ConnectToBus(Bus* ptr) { busPtr = ptr; }
	void ConnectPPU(uint8_t* ptr) { ptrToBus = ptr; }
	uint16_t ppu_read(int addr);
	void ppu_write(int addr, uint8_t data);
	uint16_t ppu_read_oam(int addr) { return primary_oam[addr]; }
	void ppu_write_oam(int addr, uint8_t data) { primary_oam[addr] = data; }
	void setScanline(int n) { ppu_scanline = n; }
	uint16_t interleave(uint16_t, uint16_t);
	void drawNametable();
	void step_ppu();
};

#endif //NESPPU_H
