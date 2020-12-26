#include "cpu.h"
#include "Bus.h"
#include "Test.h"
#include "ppu.h"
#include "mem.h"
#undef main

int main()
{
	///*
	//	LDA #$06
	//	STA $2006
	//	LDA #$00
	//	STA $2006
	//	LDA $2007
	//	LDA $2007
	//*/

	mem6502 rom_mem;
	rom_mem.load("roms/donkey_kong_edited.nes", 0x000, 0xA000);
	Bus main;
	cpu6502 cpu;
	NesPPU ppu;
	main.ConnectBus(&cpu, &ppu);
	ppu.ConnectToBus(&main);
	cpu.ConnectTotBus(&main);

	//prg_rom
	for (int i = 0x0; i < 0x4000; i++)
	{
		main.BusWrite(0x8000 + i, rom_mem[i]);
		main.BusWrite(0xC000 + i, rom_mem[i]);
	}

	//chr_rom
	int inc = 0;
	for (int i = 0x4000; i < 0x6000; i++)
	{
		ppu.ppu_write(0x0000 + inc, rom_mem[i]);
		inc++;
	}

	main.init();

	////ppu.ppu_write(0x600, 0xF);
	////cpu.setPC(0x0400);
	////cpu.write(0x400, 0x2);
	////cpu.write(0x400, 0xA9);
	////cpu.write(0x401, 0x06);
	////cpu.write(0x402, 0x8D);
	////cpu.write(0x403, 0x06);
	////cpu.write(0x404, 0x20);
	////cpu.write(0x405, 0xA9);
	////cpu.write(0x406, 0x00);
	////cpu.write(0x407, 0x8D);
	////cpu.write(0x408, 0x06);
	////cpu.write(0x409, 0x20);
	////cpu.write(0x40A, 0xAD);
	////cpu.write(0x40B, 0x07);
	////cpu.write(0x40C, 0x20);
	////cpu.write(0x40D, 0xAD);
	////cpu.write(0x40E, 0x07);
	////cpu.write(0x40F, 0x20);

	main.run();

	return 0;
}
 