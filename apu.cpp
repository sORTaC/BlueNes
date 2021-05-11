#include "apu.h"
#include "Bus.h"

void NesApu::apu_write(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
		case 0x4000:
		{
			switch ((data & 0xC0) >> 6)
			{
			case 0x00:
				pulse1_seq.sequence = 0b00000001;
				break;
			case 0x01:
				pulse1_seq.sequence = 0b00000011;
				break;
			case 0x02:
				pulse1_seq.sequence = 0b00001111;
				break;
			case 0x03:
				pulse1_seq.sequence = 0b11111100;
				break;
			}
			break;
		}
		case 0x4001:
		{	
			break;
		}
		case 0x4002:
		{
			pulse1_seq.reload = (pulse1_seq.reload & 0xff00) | data;
			break;
		}
		case 0x4003:
		{
			pulse1_seq.reload = (uint16_t)((data & 0x07)) << 8 | (pulse1_seq.reload & 0x00ff);
			pulse1_seq.timer = pulse1_seq.reload;
			break;
		}
		case 0x4015:
		{
			pulse1_enable = data & 0x01;
			break;
		}
	}
}

uint8_t NesApu::apu_read(uint16_t)
{

}

uint8_t NesApu::clock_pulse1(bool enable)
{
	if (enable)
	{
		pulse1_seq.timer--;
		if (pulse1_seq.timer == 0xffff)
		{
			pulse1_seq.timer = pulse1_seq.reload + 1;
			pulse1_seq.sequence = ((pulse1_seq.sequence & 0x0001) << 7) | ((pulse1_seq.sequence & 0x00FE) >> 1);
			pulse1_seq.output = pulse1_seq.sequence & 0x00000001;
		}
		return pulse1_seq.output;
	}
}

void NesApu::step_apu()
{
	enQuarter = false;
	enHalf = false;

	frame_clock_counter++;

	if (frame_clock_counter == 3729)
	{
		enHalf = true;
	}

	if (frame_clock_counter == 7457)
	{
		enHalf = true;
		enQuarter = true;
	}

	if (frame_clock_counter == 11186)
	{
		enQuarter = true;
	}

	if (frame_clock_counter == 14916)
	{
		enHalf = true;
		enQuarter = true;
		frame_clock_counter = 0;
	}

	if (enHalf)
	{

	}

	if (enQuarter)
	{

	}

	pulse1_sample = clock_pulse1(pulse1_enable);
}

double NesApu::getSample()
{
	return pulse1_seq.output;
}
