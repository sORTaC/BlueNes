//#include "nrom.h"
//
//nrom::nrom(cpu6502* _cpu, ppu2C02* ppu, uint8_t prg_size, std::vector<uint8_t> prg_rom, std::vector<uint8_t> chr_rom, int vertical)
//{
//	cpu = _cpu;
//	if (prg_size == 1)
//	{
//		for (int i = 0; i < (0xbfff - 0x8000); i++)
//		{
//			cpu->write(i + 0x8000, prg_rom[i]);
//			cpu->write(i + 0xC000, prg_rom[i]);
//		}
//	}
//	else if (prg_size == 2)
//	{
//		for (int i = 0; i < (0xffff - 0x8000); i++)
//		{
//			cpu->write(i + 0x8000, prg_rom[i]);
//		}
//	}
//
//	if (!vertical)
//		ppu->ppu_set_horizontal();
//	else
//		ppu->ppu_set_vertical();
//
//	for (int i = 0; i < 0x2000; i++)
//	{
//		ppu->ppuwrite(i, chr_rom[i]);
//	}
//
//}
