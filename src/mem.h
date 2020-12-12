#ifndef MEM6502_H
#define MEM6502_H

#include <vector>
#include <fstream>
#include <cstdint>

class mem6502
{
private:

	std::vector<uint8_t> mem;

public:
	mem6502();
	mem6502(int size);
	void mem6502init();
	uint8_t operator[](uint16_t)const;
	uint8_t& operator[](uint16_t);
	void load(char const*, int start, int bytes);
	uint8_t return_value(uint16_t addr);
	void contents();
};

#endif // MEM6502_H
