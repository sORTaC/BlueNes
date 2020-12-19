#ifndef CONSOLE_H
#define CONSOLE_H

#include "cpu.h"
#include "ppu.h"

class Console
{
private:
	cpu6502 cpu;
	NesPPU ppu;
public:
	Console();
	void run();
};

#endif
