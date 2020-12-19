//#include "cartridge.h"
//
//cartridge::cartridge(cpu6502* cpu_, ppu2C02* ppu_)
//{
//	cpu = cpu_;
//	ppu = ppu_;
//}
//
//void cartridge::load(const char* filepath)
//{
//	std::ifstream file(filepath, std::ios::in | std::ios::binary);
//	uint32_t name;
//	uint8_t prg_size, chr_size;
//	uint8_t flags[5];
//	uint8_t padding[5];
//	uint8_t mapper_number;
//
//	if (file.is_open())
//	{
//		printf("ROM is loaded\n");
//
//		file.read((char*)&name, sizeof(name));
//		file.read((char*)&prg_size, sizeof(prg_size));
//		file.read((char*)&chr_size, sizeof(chr_size));
//		file.read((char*)&flags, sizeof(flags));
//		file.read((char*)&padding, sizeof(padding));
//
//		prg_rom.resize((int)prg_size * 0x4000);
//		chr_rom.resize((int)chr_size * 0x2000);
//		file.read((char*)&prg_rom[0], (int)prg_size * 0x4000);
//		file.read((char*)&chr_rom[0], (int)chr_size * 0x2000);
//	}
//	else
//	{
//		printf("Error: Can't Open Rom");
//	}
//
//	mapper_number = ((flags[1] & 0b11110000)) | ((flags[0] & 0b11110000) >> 8);
//
//	switch (mapper_number)
//	{
//	case 0x00:
//		nrom(cpu, ppu, prg_size, prg_rom, chr_rom, flags[0] & 0x1);
//		break; 
//	default:
//		printf("Error: Mapper Can't Be Found");
//	}
//
//}