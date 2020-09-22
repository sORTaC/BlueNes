#pragma once

#include<iostream>
#include<cstdio>
#include<cstdlib>

typedef struct statusRegister
{
	bool negative : 1;
	bool overflow : 1;
	bool ignored : 1;
	bool brk : 1;
	bool decimal : 1;
	bool interrupt : 1;
	bool zero : 1;
	bool carry : 1;
} statusRegister;

/*typedef struct addressingMode
{
	unsigned int ZERO_PAGE : 0;
	INDEXED_ZERO_PAGE_X : 1;
	INDEXED_ZERO_PAGE_Y : 2;
	ABSOLUTE : 3;
	INDEXED_ABSOLUTE_X : 4;
	INDEXED_ABSOLUTE_Y : 5;
	IMPLIED : 6;
	ACCUMULATOR : 7;
	IMMEDIATE : 8;
	RELATIVE : 9;
	INDEXED_INDIRECT : 10;
	INDIRECT_INDEXED : 11;
	INDIRECT : 12;
} addressingMode;*/

class CPU
{
private:

	/*There is the accumulator, 2 indexes, a program counter, 
	the stack pointer, and the status register. 
	Unlike many CPU families, members do not have generic groups of registers like say, R0 through R7.*/

	/*
	MEMORY MAP:-

	$0000-$07FF 	$0800 	2KB internal RAM
    $0800-$0FFF 	$0800 	Mirrors of $0000-$07FF
	$1000-$17FF 	$0800
	$1800-$1FFF 	$0800
	$2000-$2007 	$0008 	NES PPU registers
	$2008-$3FFF 	$1FF8 	Mirrors of $2000-2007 (repeats every 8 bytes)
	$4000-$4017 	$0018 	NES APU and I/O registers
	$4018-$401F 	$0008 	APU and I/O functionality that is normally disabled. See CPU Test Mode.
	$4020-$FFFF 	$BFE0 	Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note)

	$6000-$7FFF = Battery Backed Save or Work RAM
	$8000-$FFFF = Usual ROM, commonly with Mapper Registers (see MMC1 and UxROM for example)

	Some parts of the 2 KiB of internal RAM at $0000-$07FF have predefined purposes dictated by the 6502 architecture.
	The zero page is $0000-$00FF, and the stack always uses some part of the $0100-$01FF page.

    The CPU expects interrupt vectors in a fixed place at the end of the cartridge space:

	$FFFA-$FFFB = NMI vector
	$FFFC-$FFFD = Reset vector
	$FFFE-$FFFF = IRQ/BRK vector
	*/
	uint8_t memory[UINT16_MAX];
	uint8_t ram[2046 + 1];

	uint8_t A; //Accumulator
	uint8_t X, Y; //indexes
	uint16_t PC;// program counter
	uint8_t SP; //stack pointer
	statusRegister P; //status register


	//opcodes
	void ADC();
	void AND();
	void ASL();
	void BBR();
	void BBS();
	void BCC();
	void BCS();
	void BEQ();
	void BIT();
	void BLP();
	void BMI();
	void BNE();
	void BPL();
	void BRA();
	void BRK();
	void BVC();
	void BVS();
	void CLC();
	void CLD();
	void CLI();
	void CLV();
	void CMP();
	void CPX();
	void CPY();
	void DEC();
	void DEX();
	void DEY();
	void EOR();
	void ERR();
	void INC();
	void INX();
	void INY();
	void JMP();
	void JMT();
	void JSR();
	void LDA();
	void LDX();
	void LDY();
	void LSR();
	void NOP();
	void ORA();
	void PHA();
	void PHP();
	void PHX();
	void PHY();
	void PLA();
	void PLP();
	void PLX();
	void PLY();
	void RMB();
	void ROL();
	void ROR();
	void RTI();
	void RTS();
	void SBC();
	void SEC();
	void SED();
	void SEI();
	void SMB();
	void STA();
	void STC();
	void STP();
	void STX();
	void STY();
	void STZ();
	void TAX();
	void TAY();
	void TRB();
	void TSB();
	void TSX();
	void TXA();
	void TXS();
	void TYA();
	void WAI();
};

