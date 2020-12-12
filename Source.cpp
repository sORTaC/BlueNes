#include "cpu.h"
#include "mem.h"
#include "cartridge.h"
#include "ppu.h"
#include "SDL.h"
#undef main
///*
//NOTES:-
//1.Fix memory allocation
//2.
//*/
//
int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("NES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 128 * 2, 256 * 2, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 128, 256);

	Uint32* pixels = new Uint32[128 * 256];

	mem6502 rom_mem;
	mem6502 cpu_mem;
	mem6502 ppu_mem;

	rom_mem.load("roms/donkey_kong_edited.nes", 0x000, 0xA000);

	cpu6502 cpu(cpu_mem);
	NesPPU ppu(ppu_mem, &cpu);

	//prg_rom
	for (int i = 0x0; i < 0x4000; i++)
	{
		cpu.write(0x8000 + i, rom_mem[i]);
		cpu.write(0xC000 + i, rom_mem[i]);
	}

	//chr_rom
	int inc = 0;
	for (int i = 0x4000; i < 0x6000; i++)
	{
		ppu.ppuwrite(0x0000 + inc, rom_mem[i]);
		inc++;
	}

	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 128; x++)
		{
			uint16_t addr = std::floor(x / 8 + 0.01) * 16 + std::floor(y / 8 + 0.01) * 256 + y % 8;
			uint16_t pixel_help = (uint16_t)ppu.interleave(ppu.ppuread(addr), ppu.ppuread(addr + 8));
			int pos = 7 - (x % 8);
			int opt = (pos - 1) * 2;
			uint8_t pixel = (pixel_help & (0x3 << opt)) >> opt;
			uint8_t A, R, G, B;
			A = R = G = B = 0;
			switch (pixel)
			{
			case 0:
				break;
			case 1:
				R = 255;
				break;
			case 2:
				R = G = B = 150;
				break;
			case 3:
				R = G = B = 200;
				break;
			}
			int color = 0;
			color |= (A << 24) + (R << 16) + (G << 8) + (B << 0);
			pixels[y * 128 + x] = color;
		}
	}

	cpu.runfor(50000000);
	for (int i = -1; i < 260; i++)
	{
		ppu.Render(i);
	}
	for (int i = 0; i < 0x0400; i++)
	{
		printf("%x ", ppu.ppuread(0x2000 + i));
	}
	//uint16_t old_pc = cpu.getPC();
	//cpu.setPC(old_pc);

	//SDL_Event event;
	//bool quit = false;
	//while (!quit)
	//{
	//	SDL_UpdateTexture(texture, 0, pixels, 128 * sizeof(Uint32));
	//	while (SDL_PollEvent(&event))
	//	{
	//		if (event.type == SDL_QUIT)
	//		{
	//			quit = true;
	//		}
	//	}
	//	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	//	SDL_RenderClear(renderer);
	//	SDL_RenderCopy(renderer, texture, 0, 0);
	//	SDL_RenderPresent(renderer);
	//}

	//delete[] pixels;
	//SDL_DestroyWindow(window);
	//SDL_DestroyRenderer(renderer);
	//SDL_DestroyTexture(texture);
	//window = NULL;
	//renderer = NULL;
	//texture = NULL;
	//SDL_Quit();
	return 0;
}






//int main()
//{
//	mem6502 mem;
//	cpu6502 cpu(mem);
//	//mem6502 ppumem(0x4000);
//	//ppu2C02 ppu(ppumem, &cpu);
//	////cartridge cartridge(&cpu, &ppu);
//	////cartridge.load("roms/donkey_kong");
//	return 0;
//}


//#define BLAARG

//BLAARG TEST INFO - PASS OR FAIL
//01-basics - passed
//02-implied - passed
//03-immediate - passed
//04-zero_page - passed
//05-zp_xy - idk
//06-abs - idk
//07-abs_xy - idk
//08-ind_x - idk
//09-ind_y - idk
//10-branches - passed
//11-stack - passed
//12-jmp_jsr - passed
//13-rts - passed
//14-rti - failed
//15-brk - passed
//16-special - passed

//#define KLAUS2M5

//int main()
//{
//
//#ifdef BLAARG
//
//	mem6502 mem;
//	cpu6502 cpu(mem);
//	cartridge cartridge(&cpu, &ppu);
//	cartridge.load("logandtests/rom_singles/09-ind_y.nes");
//	cpu.write(0xffff, 0xe2);
//	//cpu.mem_dump(0x8000, 0xffff);
//	cpu.reset();
//	cpu.runfor(50000000);
//	for (int i = 0; i < 30; i++)
//	{
//		printf("%c ", cpu.read(0x6000 + i));
//	}
//
//#endif
//
//
//#ifdef KLAUS2M5
//	mem6502 mem;
//	mem.load("logandtests/6502_functional_test.bin", 0x000, 65536);
//	cpu6502 cpu(mem);
//	cpu.setPC(0x0400);
//	cpu.runfor(100000000);
//	cpu.state_dump();
//
//#endif
//
//	//mem6502 cpumem;
//	//cpu6502 cpu(cpumem);
//	//cpu.write(0x2007, 0x50);
//	//cpu.mem_dump(0x2000, 0x3fff);
//
//	return 0;
//}

//MY TEST
//	//mem.load("logandtests/code2.bin", 0x0600, 148);
