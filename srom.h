#ifndef  SROM
#define SROM

#include "mem.h"
#include <vector>
class srom
{
private:
	const char* file = NULL;
	mem6502 header;
	mem6502 CartridgeData;
	uint8_t BankSelect;
	uint8_t prgRomSize;
	uint8_t control;
	uint8_t chrLO;
	uint8_t chrHI;
	uint8_t CHR;
	uint8_t prgLO;
	uint8_t prgHI;
	uint8_t PRG;
	uint8_t mregister;
	uint8_t counter;
public:
	uint8_t mapper;
	uint8_t prgRamSize;
	uint8_t chrRomSize;
	void mapperLoad(const char*);
	uint8_t mapperRead(int);
	void mapperWrite(int, int);
	uint8_t mapperReadCHR(int);
	uint8_t mapperReadRAM(int);
};

#endif // ! SROM


