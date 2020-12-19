#include "Test.h"

Test::Test()
{
	cpu.ConnectCPU(ram);
	ppu.ConnectPPU(ram);
}

