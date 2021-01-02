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
	ptrToBus = NULL;
	ppu_cycles = 0;
	ppu_scanline = 0;
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
	else if (addr >= 0x3f00 && addr <= 0x3F1F)
	{
		ppu_ram[addr] = data;
		ppu_ram[addr + 0x20] = data;
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

void NesPPU::step_ppu()
{
	if (ppu_scanline >= 0 && ppu_scanline < 240)
	{

	}
	else if (ppu_scanline == 241 && ppu_cycles == 1)
	{
		busPtr->set_vblank(1);
		busPtr->set_nmi(1);
		drawNametable();
		updateWindow();
	}
	else if (ppu_scanline == 261)
	{
		busPtr->set_vblank(0);
		busPtr->set_nmi(0);
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

void NesPPU::updateWindow()
{
	SDL_UpdateTexture(texture, 0, pixels, 256 * sizeof(Uint32));
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, 0, 0);
	SDL_RenderPresent(renderer);
}

void NesPPU::drawNametable()
{
	for (int y = 0; y < 240; y++)
	{
		//background
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
			int bkg_color = (pallete[ppu_read(0x3f00 + c + pixel)]);
			(c + pixel) != 0 ? (bkg_color += (0xf << 24)) : bkg_color;
			pixels[y * 256 + x] = bkg_color;
		}
		//sprite
		for (int i = 0; i < 63; i++)
		{
			uint8_t y = primary_oam[i * 4 + 0];
			uint8_t t = primary_oam[i * 4 + 1];
			uint8_t a = primary_oam[i * 4 + 2];
			uint8_t x = primary_oam[i * 4 + 3];

			bool flip_horizontal = a & 0x40 ? 1 : 0;
			bool flip_vertical = a & 0x80 ? 1 : 0;
			int tile = busPtr->return_spr() + t * 16;
			uint8_t priority = (a & 0x20);
			uint8_t sprite_pixel = 0;

			for (int k = 0; k < 8; k++)
			{
				int tile_x = ppu_read(tile + k);
				int tile_y = ppu_read(tile + k + 8);
				uint16_t sprite_pixel_help = interleave(tile_x, tile_y);

				for (int l = 0; l < 8; l++)
				{

					if (flip_horizontal & flip_vertical)
					{

					}
					else if (flip_vertical)
					{

					}
					else if (flip_horizontal)
					{
						int pos = (l % 8);
						int opt = (pos) * 2;
						sprite_pixel = (sprite_pixel_help & (0x3 << opt)) >> opt;
					}
					else//no flip
					{
						int pos = 7 - (l % 8);
						int opt = (pos) * 2;
						sprite_pixel = (sprite_pixel_help & (0x3 << opt)) >> opt;
					}
					if (sprite_pixel != 0)
					{
						uint8_t background_channel = pixels[(y + k + 1) * 256 + (x + l)] >> 24;
						if (background_channel == 0)
						{
							pixels[(y + k + 1) * 256 + (x + l)] = pallete[ppu_read(0x3f10 + (a & 0x3) * 4 + sprite_pixel)];
						}
						else if (background_channel != 0)
						{
							if (priority == 0)
							{
								pixels[(y + k + 1) * 256 + (x + l)] = pallete[ppu_read(0x3f10 + (a & 0x3) * 4 + sprite_pixel)];
							}
						}
					}
				}
			}
		}
	}
}
