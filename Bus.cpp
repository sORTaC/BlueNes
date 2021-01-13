#include "Bus.h"

Bus::Bus()
{
	cpu = NULL;
	ppu = NULL;
	keys = (uint8_t*)SDL_GetKeyboardState(NULL);
	for (int i = 0; i < 0xffff; i++)
	{
		ram[i] = 0xEA;
	}
}

void Bus::ConnectBus(cpu6502* cpuPtr, NesPPU* ppuPtr)
{
	cpu = cpuPtr;
	ppu = ppuPtr;
}

uint8_t Bus::ask_cpu(uint16_t addr)
{
	return cpu->read(addr);
}

uint16_t Bus::BusRead(uint16_t addr)
{
	uint8_t data = 0;

	if (addr >= 0x000 && addr < 0x2000)
	{
		data = ram[addr & 0x7FF];
	}
	else if (addr >= 0x2000 && addr < 0x4000)
	{
		data = ppu->ppu_read_registers(addr);
	}
	else if (addr >= 0x4000 && addr < 0x4020)
	{
		if (addr >= 0x4016 && addr <= 0x4017)
		{

		}
		else
		{
			data = ram[addr];
		}
	}
	else
	{
		data = ram[addr];
	}

	return data;
}

void Bus::BusWrite(uint16_t addr, uint8_t data)
{
	if (addr >= 0x000 && addr < 0x2000)
	{
		ram[addr & 0x7FF] = data;
	}
	else if (addr >= 0x2000 && addr < 0x4000)
	{
		ppu->ppu_write_registers(addr, data);
	}
	else if (addr >= 0x4000 && addr < 0x4020)
	{
		if (addr == 0x4014)
		{
			ppu->ppu_write_4014(data);
		}
		else if(addr >= 0x4016 && addr <= 0x4017)
		{

		}
		else
		{
			ram[addr] = data;	
		}
	}
	else
	{
		ram[addr] = data;
	}
}

void Bus::init()
{
	cpu->reset();
	ppu->ppu_powerup();
}

void Bus::run()
{
	int cycles = 0;
	while (true)
	{

		if (ppu->check_for_nmi()) {
			cpu->nmi();
			ppu->set_nmi_occured(0);
		}

		cycles = cpu->step_instruction();

		for (int i = 0; i < cycles * 3; i++) {
			ppu->step_ppu();
		}

		cycles = 0;
	}
}

