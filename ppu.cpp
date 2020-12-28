#include "Bus.h"
#include "ppu.h"

uint32_t pallete[64] = {
	0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400, 0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
	0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10, 0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
	0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044, 0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
	0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8, 0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
};

NesPPU::NesPPU()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("NES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 240);
	pixels = new Uint32[240 * 256];
}

NesPPU::~NesPPU()
{
	delete[] pixels;
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	window = NULL;
	renderer = NULL;
	texture = NULL;
	SDL_Quit();
}

void NesPPU::ppu_write(int addr, uint8_t data)
{
	if (addr >= 0x2000 && addr <= 0x3000)
	{
		if (horizontal_mirroring)
		{
			ppu_ram[addr] = data;
			ppu_ram[addr + 0x400] = data;
		}
		else
		{
			ppu_ram[addr] = data;
			ppu_ram[addr + 0x800] = data;
		}
	}
	else
		ppu_ram[addr] = data;
}

uint16_t NesPPU::ppu_read(int addr) 
{
	return ppu_ram[addr]; 
}

uint16_t NesPPU::interleave(uint16_t byte_0, uint16_t byte_1)
{
    int final = 0;
    for (int i = 0; i < 8; i++)
    {
        int x_masked_i = (byte_0 & (1 << i));
        int y_masked_i = (byte_1 & (1 << i));

        final |= (x_masked_i << i);
        final |= (y_masked_i << (i + 1));
    }
    return final;
}

void NesPPU::ppu_set_vblank()
{
    busPtr->set_vblank();
    busPtr->set_nmi();
}

void NesPPU::ppu_clear_vblank()
{
    busPtr->clr_vblank();
    busPtr->clear_nmi();
}

void NesPPU::step_ppu()
{
    if (ppu_scanline >= 0 && ppu_scanline < 240)
    {
		SDL_UpdateTexture(texture, 0, pixels, 256 * sizeof(Uint32));
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, 0, 0);
		SDL_RenderPresent(renderer);
    }
    else if (ppu_scanline == 241 && ppu_cycles == 1)
    {
        busPtr->set_vblank();
        busPtr->set_nmi();
        drawNametable();
    }
    else if (ppu_scanline == 261)
    {
        busPtr->clr_vblank();
        busPtr->clear_nmi();
        ppu_scanline = 0;
    }

    ppu_cycles++;

    if (ppu_cycles > 340)
    {
        ppu_cycles = 0;
        ppu_scanline++;
    }
}

void NesPPU::ppu_powerup()
{
    ppu_scanline = 0;
    ppu_cycles = 0;
}

void NesPPU::drawNametable()
{

	for (int y = 0; y < 960; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			//nametable byte
			uint16_t addr_tile = 0x2000 + (32 * (y / 8)) + (x / 8);
			uint16_t tile_nr = ppu_read(addr_tile);
			int addr = busPtr->return_bkg() + (tile_nr * 0x10) + (y % 8);
			uint16_t pixel_help = (uint16_t)interleave(ppu_read(addr), ppu_read(addr + 8));
			int pos = 7 - (x % 8);
			int opt = (pos) * 2;
			uint8_t pixel = (pixel_help & (0x3 << opt)) >> opt;

			//attrib byte
			int attrib_byte = 0x2000 + 0x03C0 + (y / 32) * 8 + (x / 32);
			int pal = ppu_read(attrib_byte);
			int location_x = (x / 16) & 1;
			int location_y = (y / 16) & 1;
			int location = (location_y << 1) | location_x;
			int c = pixel == 0 ? 0 : ((pal >> (location * 2)) & 0b11) * 4;

			//store
			pixels[y * 256 + x] = (pallete[ppu_read(0x3f00 + c + pixel)]);
		}
	}
}
