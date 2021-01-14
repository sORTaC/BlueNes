#include "cpu.h"
#include "Bus.h"
#include "ppu.h"
#include "mem.h"
#undef main

int main()
{

	mem6502 rom_mem;
	rom_mem.load("roms/smb_edited.nes", 0x000, 0xA000);
	Bus main;
	cpu6502 cpu;
	NesPPU ppu;
	main.ConnectBus(&cpu, &ppu);
	ppu.ConnectToBus(&main);
	cpu.ConnectTotBus(&main);

	//prg_rom
	for (int i = 0x0; i < 0x8000; i++)
	{
		main.BusWrite(0x8000 + i, rom_mem[i]);
		main.BusWrite(0xC000 + i, rom_mem[i]);
	}

	//chr_rom
	int inc = 0;
	for (int i = 0x8000; i < 0xA000; i++)
	{
		ppu.ppu_write(0x0000 + inc, rom_mem[i]);
		inc++;
	}

	main.init();
	main.run();

	return 0;
}
