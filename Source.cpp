#include "cpu.h"
#include "Bus.h"
#include "ppu.h"
#include "mem.h"
#undef main

int main()
{
	Bus main;
	cpu6502 cpu;
	NesPPU ppu;
	main.ConnectBus(&cpu, &ppu);
	ppu.ConnectToBus(&main);
	cpu.ConnectTotBus(&main);
	main.BusMapperSet();
	main.init();
	main.run();
	return 0;
}
