#include "Bus.h"

Bus::Bus()
{
	cpu = NULL;
	ppu = NULL;
	ppu_buffer = 0;
	temporary_register = 0;
	vram_addr = 0;
	vram_inc = 0;
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

uint16_t Bus::BusRead(uint16_t addr)
{
	uint8_t data = ram[addr];
	if (addr >= 0x000 && addr < 0x2000)
	{
		return ram[addr & 0x7FF];
	}
	else if (addr >= 0x2000 && addr < 0x4000)
	{
		switch (addr & 0x2007)
		{
		case 0x2000:
			break;
		case 0x2001:
			break;
		case 0x2002:
			//clear nmi_occured
			address_latch = true;
			nmi_occured = false;
			if (vblank)
				data = 0x80;
			else
				data = 0;
			break;
		case 0x2003:
			break;
		case 0x2004:
			break;
		case 0x2005:
			break;
		case 0x2006:
			break;
		case 0x2007:
		{
			data = ppu_buffer;
			ppu_buffer = (uint8_t)ppu->ppu_read(vram_addr);
			if (vram_addr > 0x3f00)
				data = ppu_buffer;
			//increment vram addr per read/write of ppudata
			vram_addr += vram_inc;
			break;
		}
		}
	}
	else if (addr >= 0x4000 && addr < 0x4020)
	{
		//printf("\nError! Reading from APU region\n");
		return ram[addr];
	}
	else
	{
		//printf("\nError! Reading from Cartridge region\n");
		return ram[addr];
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
		switch (addr & 0x2007)
		{
		case 0x2000:
			nmi_output = (data & 0x80) ? true : false;
			vram_inc = (data & 0x4) ? 32 : 1;
			background_table = (data & 0x10) ? 0x1000 : 0x000;
			break;
		case 0x2001:
			break;
		case 0x2002:
			break;
		case 0x2003:
			break;
		case 0x2004:
			break;
		case 0x2005:
			break;
		case 0x2006://ppuaddr
		{
			//$2006 holds no memory
			if (address_latch)
			{
				//high byte
				temporary_register &= 0x00ff;
				temporary_register |= (data & 0x3F) << 8;
				address_latch = false;
			}
			else
			{
				//low byte
				temporary_register &= 0xff00;
				temporary_register |= data;
				address_latch = true;
				vram_addr = temporary_register;
			}
			break;
		}
		case 0x2007://ppudata
		{
			if (vram_addr == (0x2400 - 0x1F))
			{
				if (frame == 3)
				{
					printf("\nDATA FOR NT:%x\n", data);
				}
			}
			ppu->ppu_write(vram_addr, data);
			// increment vram addr per read/write of ppudata
			vram_addr += vram_inc;
			break;
		}
		}
		ram[addr] = data;
	}
	else if (addr >= 0x4000 && addr < 0x4020)
	{
		//printf("\nError! Writing to APU region\n");
		ram[addr] = data;
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
		if (nmi_output && nmi_occured) {
			cpu->nmi();
			nmi_output = false;
		}
		cycles = cpu->step_instruction();
		//cpu->printIns();
		for (int i = 0; i < cycles * 3; i++) {
			ppu->step_ppu();
		}
		frame++;
	}
}

