#ifndef NESPPU_H
#define NESPPU_H

#include <array>
#include <iostream>

class Bus;

class NesPPU
{
private:
	uint8_t ppu_ram[0x4000];
	uint8_t primary_oam[64 * 4];

	int ppu_cycles, ppu_scanline;
	bool horizontal_mirroring = true;
	bool nmi_occured, nmi_output;
	bool sprite_0_hit = false;
	bool vblank;

	uint16_t dma_page;
	uint16_t background_table;
	uint16_t sprite_table;
	uint8_t ppu_buffer;
	uint8_t v_inc;
	uint8_t oam_addr;
	uint16_t v;
	uint16_t t;
	uint8_t x;
	bool w;

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

	void ConnectToBus(Bus* ptr) { busPtr = ptr; }
	void ConnectPPU(uint8_t* ptr) { ptrToBus = ptr; }

	uint16_t interleave(uint16_t, uint16_t);

	void ppu_write(int addr, uint8_t data);
	void ppu_write_4014(uint8_t data);
	void ppu_write_registers(uint16_t addr, uint8_t data);
	void ppu_write_oam(int addr, uint8_t data) { primary_oam[addr] = data; }
	uint8_t ppu_read_registers(uint16_t addr);
	uint16_t ppu_read(int addr);
	uint16_t ppu_read_oam(int addr) { return primary_oam[addr]; }

	bool check_for_nmi() { return (nmi_occured && nmi_output); }
	void set_nmi_output(bool m) { nmi_output = m; }

	void drawBackground();
	void drawSprites();
	void updateWindow();
	void step_ppu();
};

#endif //NESPPU_H



