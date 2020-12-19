#include "Bus.h"
#include "ppu.h"

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

void NesPPU::ppu_set_vblank()
{
    busPtr->BusWrite(0x2002, busPtr->BusRead(0x2002) + 0x80);
}

void NesPPU::ppu_clear_vblank()
{
    busPtr->BusWrite(0x2002, busPtr->BusRead(0x2002) & 0x7F);
}

void NesPPU::step_ppu()
{
    nmi_occured = true;
    if (ppu_scanline >= 0 && ppu_scanline < 240)
    {

    }
    else if (ppu_scanline == 241 && ppu_cycles == 1)
    {
        ppu_set_vblank();
        nmi_occured = true;
    }
    else if (ppu_scanline == 261)
    {
        ppu_clear_vblank();
        nmi_occured = false;
        ppu_scanline = 0;
    }
    ppu_cycles++;
    if (ppu_cycles > 340)
    {
        ppu_cycles = 0;
        ppu_scanline++;
    }
}
