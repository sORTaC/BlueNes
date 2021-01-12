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
	uint8_t secondary_oam[8 * 4];

	int ppu_cycles, ppu_scanline;
	int sprite_count;
	bool horizontal_mirroring = false;
	bool nmi_occured, nmi_output;
	bool sprite_0_hit = false;
	bool vblank;

	int nametable_byte;
	int attribute_byte;
	int low_bg_tile;
	int hi_bg_tile;

	uint8_t base_nt;
	uint8_t sprite_hit_cycle;
	uint16_t background_table;
	uint16_t sprite_table;
	uint8_t ppu_buffer;
	uint8_t v_inc;
	uint8_t oam_addr;
	uint16_t v;
	uint16_t t;
	uint8_t fine_x;
	bool w;

	uint8_t scroll_x;
	uint8_t scroll_y;

	Bus* busPtr;
	uint8_t* ptrToBus;
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

	void increment_x();
	void increment_y();
	void copy_x();
	void copy_y();


	void ppu_write(int addr, uint8_t data);
	void ppu_write_4014(uint8_t data);
	void ppu_write_registers(uint16_t addr, uint8_t data);
	uint8_t ppu_read_registers(uint16_t addr);
	uint16_t ppu_read(int addr);

	bool check_for_nmi() { return (nmi_occured && nmi_output); }
	void set_nmi_occured(bool m) { nmi_occured = m; }

	void drawBackgroundLines(int);
	void evaluateSprites(int);
	void drawSpriteLines(int);
	void updateWindow();
	void step_ppu();
};

#endif //NESPPU_H
