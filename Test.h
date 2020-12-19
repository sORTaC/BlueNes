#include "cpu.h"
#include "ppu.h"
class Test
{
private:
	uint8_t ram[0xffff];
	cpu6502 cpu;
	NesPPU ppu;
public:
	Test();
	void write(uint16_t addr, uint8_t data) { cpu.write(addr, data); }
	uint8_t read(uint16_t addr) { return ram[addr]; }
};

