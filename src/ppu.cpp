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

void NesPPU::ppu_write_4014(uint8_t data)
{
	dma_page = data;
	int oam_addr_cpy = oam_addr;
	for (int dma_addr = 0; dma_addr < 256; dma_addr++)
	{
		int addr = dma_addr + oam_addr_cpy;
		if (addr > 256)
			oam_addr_cpy = 0;
		//need to implement cpu wait
		int data = busPtr->ask_cpu(dma_page << 8 | dma_addr);
		primary_oam[addr] = data;
	}
}

void NesPPU::y_increment()
{
	if ((v & 0x7000) != 0x7000)
	{
		v += 0x1000;
	}
	else
	{
		v &= ~0x7000;                    
		int y = (v & 0x03E0) >> 5;      
		if (y == 29)
		{
			y = 0;                        
			v ^= 0x0800;
		}                
		else if (y == 31)
		{
			y = 0;
		}
		else
		{
			y += 1;                    
			v = (v & ~0x03E0) | (y << 5);
		}     
	}
}

void NesPPU::x_increment()
{
	if ((v & 0x001F) == 31)
	{
		v &= ~0x001F;
		v ^= 0x0400;
	}
	else
		v += 1;
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
		switch (addr)
		{
		case 0x3f00:
			ppu_ram[0x3f10] = data;
			break;
		case 0x3f04:
			ppu_ram[0x3f14] = data;
			break;
		case 0x3f08:
			ppu_ram[0x3f18] = data;
			break;
		case 0x3f0c:
			ppu_ram[0x3f1c] = data;
			break;
		}
		switch (addr)
		{
		case 0x3f10:
			ppu_ram[0x3f00] = data;
			break;
		case 0x3f14:
			ppu_ram[0x3f04] = data;
			break;
		case 0x3f18:
			ppu_ram[0x3f08] = data;
			break;
		case 0x3f1c:
			ppu_ram[0x3f0c] = data;
			break;
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

void NesPPU::step_ppu()
{
	if (ppu_scanline >= 0 && ppu_scanline < 240)
	{

	}
	else if (ppu_scanline == 241 && ppu_cycles == 1)
	{
		vblank = true;
		nmi_occured = true;
		drawBackground();
		drawSprites();
		updateWindow();
	}
	else if (ppu_scanline == 261)
	{
		vblank = false;
		nmi_occured = false;
		sprite_0_hit = false;
		ppu_scanline = 0;
		if (ppu_cycles >= 280 && ppu_cycles <= 304)
		{
			v += ((t >> 5) & 0b111) << 5;
			v += ((t >> 8) & 0b11) << 8;
			v += ((t >> 11) & 0b1111) << 11;
		}
	}

	if ((ppu_cycles <= 256) || (ppu_cycles >= 328))
	{
		if (ppu_cycles == 256)
			y_increment();

		if (ppu_cycles % 8 == 0)
		{
			x_increment();
		}
	}

	if (ppu_cycles == 257)
	{
		v += (t & 0b11111);
		v += ((t >> 11) & 0x1) << 11;
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

uint8_t NesPPU::ppu_read_registers(uint16_t addr)
{
	uint8_t data;

	switch (addr & 0x2007)
	{
	case 0x2000:
		break;
	case 0x2001:
		break;
	case 0x2002:
		//clear nmi_occured
		w = true;
		nmi_occured = false;

		if (vblank)
			data = 0x80;
		else
			data = 0;

		if (sprite_0_hit)
			data += 0x40;

		break;
	case 0x2003:
		break;
	case 0x2004:
		if (vblank)
			return primary_oam[oam_addr]; 
		break;
	case 0x2005:
		break;
	case 0x2006:
		break;
	case 0x2007:
	{
		data = ppu_buffer;
		ppu_buffer = (uint8_t)ppu_read(v);
		if (v > 0x3f00)
			data = ppu_buffer;
		//increment vram addr per read/write of ppudata
		v += v_inc;
		break;
	}
	}
}

void NesPPU::ppu_write_registers(uint16_t addr, uint8_t data)
{
	switch (addr & 0x2007)
	{
	case 0x2000:
		v_inc = (data & 0x4) ? 32 : 1;
		sprite_table = (data & 0x8) ? 0x1000 : 0x000;
		background_table = (data & 0x10) ? 0x1000 : 0x000;
		nmi_output = (data & 0x80) ? true : false;
		t += (data & 0b11) << 10;  
		break;
	case 0x2001:
		break;
	case 0x2002:
		oam_addr++;
		break;
	case 0x2003:
		oam_addr = data;
		break;
	case 0x2004:
		primary_oam[oam_addr] = data;
		oam_addr++;
		break;
	case 0x2005:
		if (w)
		{
			t += (data >> 3);
			w = false;
		}
		else
		{
			t += ((data >> 3) & 0b111) << 5;
			t += (data & 0b111) << 12;
			t += (data >> 6) << 8;
			w = true;
		}
		break;
	case 0x2006://ppuaddr
	{
		//$2006 holds no memory
		if (w)
		{
			//high byte
			t &= 0x00ff;
			t |= (data & 0x3F) << 8;
			w = false;
		}
		else
		{
			//low byte
			t &= 0xff00;
			t |= data;
			w = true;
			v = t;
		}
		break;
	}
	case 0x2007://ppudata
	{
		ppu_write(v, data);
		// increment vram addr per read/write of ppudata
		v += v_inc;
		break;
	}
	}
}

void NesPPU::drawBackground()
{
	for (int y = 0; y < 256; y++)
	{
		//background
		for (int x = 0; x < 240; x++)
		{
			//nametable byte
			uint16_t addr_tile = 0x2000 | (v & 0x0FFF);
			uint16_t tile_nr = ppu_read(addr_tile);
			int addr = background_table + (tile_nr * 0x10) + (y % 8);
			uint16_t pixel_help = (uint16_t)interleave(ppu_read(addr), ppu_read(addr + 8));
			int pos = 7 - (x % 8);
			int opt = (pos) * 2;
			uint8_t pixel = (pixel_help & (0x3 << opt)) >> opt;

			//attrib byte
			int attrib_byte = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
			int pal = ppu_read(attrib_byte);
			int location = ((v & 0x64) << 1) | (v & 0x2);
			int c = pixel == 0 ? 0 : ((pal >> (location * 2)) & 0b11) * 4;

			//store
			int bkg_color = (pallete[ppu_read(0x3f00 + c + pixel)]);
			(c + pixel) != 0 ? (bkg_color += (0xf << 24)) : bkg_color;
			pixels[y * 256 + x] = bkg_color;
		}
	}
}

void NesPPU::drawSprites()
{
	for (int i = 63; i >= 0; i--)
	{
		bool quit = false;

		uint8_t y = primary_oam[i * 4 + 0];
		uint8_t t = primary_oam[i * 4 + 1];
		uint8_t a = primary_oam[i * 4 + 2];
		uint8_t x = primary_oam[i * 4 + 3];

		if (x >= 0 && x <= (256 - 8))
			quit = false;
		else
			quit = true;

		if (y >= 0 && y <= (240 - 8))
			quit = false;
		else
			quit = true;

		bool flip_horizontal = a & 0x40 ? 1 : 0;
		bool flip_vertical = a & 0x80 ? 1 : 0;
		int tile = sprite_table + t * 16;
		uint8_t priority = (a & 0x20);
		uint8_t sprite_pixel = 0;

		if (!quit)
		{
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
							if (!(sprite_0_hit))
							{
								sprite_0_hit = true;
							}

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
