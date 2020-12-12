#include "ppu.h"

NesPPU::NesPPU(mem6502 mem, cpu6502* cpu_)
{
    ppumem = mem;
    cpu = cpu_;
}

void NesPPU::ppuwrite(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0x2000://ppuctrl
    {
        nmi_output = data & 0x80;//set nmi output to bit 7
        ppumem[addr] = data;
        break;
    }
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
            temporary_register += data << 8;
            address_latch = false;
        }
        else
        {
            temporary_register += data;
            address_latch = true;
            effective_register = temporary_register;
        }
        break;
    }
    case 0x2007://ppudata
    {
        // increment vram addr per read/write of ppudata
        cpu->read(0x2000) & 0x3 ? vram_addr += 32 : vram_addr += 1;
        cpu->write(addr, data);
        break;
    }
    case 0x4014:
        break;
    }

    ppumem[addr] = data;

    //need to implement mirroring
    if (addr >= 0x2000 && addr <= 0x2EFF)
    {
        cpu->write(0x3000 + (addr - 0x2000), data);
    }

    if (addr >= 0x3F00 && addr <= 0x3F1F)
    {
        cpu->write(0x3F20 + (addr - 0x3F00), data);
    }
}

uint16_t NesPPU::ppuread(uint16_t addr)
{
    //from addr in ram, read data
    uint16_t data = ppumem[addr];
    switch (addr)
    {
    case 0x2001:
        break;
    case 0x2002:
    {
        //return the old status of nmi occured in bit_7
        data = data + (nmi_occured << 7);
        nmi_occured = false;
        address_latch = false;
        break;
    }
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
        ppu_buffer = ppuread(vram_addr);
        if (vram_addr > 0x3f00)
            data = ppu_buffer;
        //increment vram addr per read/write of ppudata
        cpu->read(0x2000) & 0x3 ? vram_addr += 32 : vram_addr += 1;
        break;
    }
    }
    return data;
}

uint16_t NesPPU::interleave(uint16_t byte_0, uint16_t byte_1)
{
    int final = 0;
    for (int i = 0; i < 8; i++)
    {
        int x_masked_i = (byte_0 & (1 << i));
        int y_masked_i = (byte_1 & (1 << i));

        final |= (x_masked_i << i);
        final |= (y_masked_i << (i + 1));
    }
    return final;
}

void NesPPU::Render(int scanline)
{
    //skipped tick on odd cycle by jumping from (339, 261) to (0,0)
    if (scanline == -1)
    {
        cpu->write(ppustatus, cpu->read(ppustatus) & 0x7F); //clears nmi flag
        nmi_occured = false;

    }

    if (scanline >= 0 && scanline <= 239)
    {

    }

    if (scanline >= 240 && scanline <= 260)
    {
        if (scanline == 241)
        {
            cpu->write(ppustatus, cpu->read(ppustatus) + 0x80); //sets nmi flag
            nmi_occured = true;
        }

        if (nmi_occured && nmi_output)
        {
            cpu->nmi();
        }
    }
}

uint16_t NesPPU::FetchNT(int x, int y)
{
    uint16_t BaseAddress = 0;
    uint16_t FinalAddress = 0;
    uint16_t Offset = 0;
    switch (cpu->read(ppuctrl) & 0x3)
    {
    case 0:
        BaseAddress = 0x2000;
    case 1:
        BaseAddress = 0x2400;
    case 2:
        BaseAddress = 0x2800;
    case 3:
        BaseAddress = 0x2C00;
    default:
        printf("\nError in NameTable\n");
    }
    Offset = 32 * (y / 8) + x / 8;
    FinalAddress = BaseAddress + Offset;
    return FinalAddress;
}

uint16_t NesPPU::FetchAT(int x, int y)
{
    uint16_t BaseAddress = 0;
    uint16_t FinalAddress = 0;
    uint16_t Offset = 0;
    switch (cpu->read(ppuctrl) & 0x3)
    {
    case 0:
        BaseAddress = 0x23C0;
        break;
    case 1:
        BaseAddress = 0x27C0;
        break;
    case 2:
        BaseAddress = 0x2BC0;
        break;
    case 3:
        BaseAddress = 0x2FC0;
        break;
    default:
        printf("\nError in NameTable\n");
    }
    Offset = 8 * (y / 32) + x / 32;
    FinalAddress = BaseAddress + Offset;
    return FinalAddress;
}
