#include "apu.h"
#include "Bus.h"

uint8_t len_table[16][2] =
{
	{0x0A, 0xFE},
	{0x14, 0x02},
	{0x28, 0x04},
	{0x50, 0x06},
	{0xA0, 0x08},
	{0x3C, 0x0A},
	{0x0E, 0x0C},
	{0x1A, 0x0E},
	{0x0C, 0x10},
	{0x18, 0x12},
	{0x30, 0x14},
	{0x60, 0x16},
	{0xC0, 0x18},
	{0x48, 0x1A},
	{0x10, 0x1C},
	{0x20, 0x1E}
};

uint8_t duty_table[4][8] =
{
	{0, 1, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 0, 0, 0, 0, 0},
	{0, 1, 1, 1, 1, 0, 0, 0},
	{1, 0, 0, 1, 1, 1, 1, 1}
};

void NesApu::apu_write(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000:
		vol = data & 0xF;
		env_disable  = (data >> 4) & 0x1;
		loop_disable = (data >> 5) & 0x1;
		len_disable  = (data >> 5) & 0x1;
		duty = (data >> 6) & 0x3;
		break;
	case 0x4001:
		write_to_sweep_register = true;
		sweep_shift = data & 0x7;
		sweep_negate = (data >> 3) & 0x1;
		sweep_period = (data >> 4) & 0x7;
		sweep_enable = ((data >> 7) & 0x1) && (sweep_shift != 0);
		break;
	case 0x4002:
		period = data;
		break;
	case 0x4003:
		write_to_fourth_channel = true;
		period |= ((data & 0x7) << 3);
		//a write to the channel's 4th register immediately loads it with the value from len_table
		if (SQ1_enable)
		{
			length_index = (data >> 3) & 0x1F;
			len_counter = len_table[(length_index >> 4) & 0xF][(length_index >> 3) & 0x1];
		}
		duty_counter = 0;
		freq_counter = period;
		break;
	case 0x4015:
		SQ1_enable = data & 0x1;
		SQ2_enable = data & 0x2;
		Triangle_enable = data & 0x4;
		Noise_enable = data & 0x8;
		DMC_enable = data & 0x10;
		if (!SQ1_enable) { len_counter = 0; }
	case 0x4017:
		mode = data & 0x80;
		irq_disable = data & 0x40;
		if (mode)/*mode 1 is immediately clocked*/
		{
			clock_Sweep();
			clock_Envelope();
			clock_LengthCounter();
		}
		if (irq_disable) {
			irq_pending = false;
		}
		break;
	default:
		printf("Addr here is not handled yet. Only working with sq1 right now =)");
		break;
	}
}

NesApu::NesApu()
{
	busPtr = NULL;
	cpu_cycles = 0;
	cycles_to_next_sequence = 0;
}

uint8_t NesApu::apu_read(uint16_t addr)
{
	if (addr == 0x4015)
	{
		uint8_t output = 0;
		if (len_counter != 0) output |= 0x1;
		if (irq_pending) output |= 0x40;
		irq_pending = false;
		return output;
	}
	else
	{
		printf("Only reads from address 0x4015, Err in apu_read in apu.cpp| return 0");
		return 0;
	}
}

void NesApu::clock_FrameSequencer()
{
	if (!mode)
	{
		//4-step sequence

		switch (step)
		{
		case 0:
			clock_Envelope();
			break;
		case 1:
			clock_Sweep();
			clock_LengthCounter();
			clock_Envelope();
			break;
		case 2:
			clock_Envelope();
			break;
		case 3:
			if (!irq_disable)
				irq_pending = true;
			clock_Sweep();
			clock_Envelope();
			clock_LengthCounter();
			//wrap back to 0
			step = 0;
			return;
		}
	}
	else if (mode)
	{
		//5-step sequence
		/*
			Clocked immediately when mode 5
		*/
		switch (step)
		{
		case 0:
			clock_Envelope();
			break;
		case 1:
			clock_Sweep();
			clock_LengthCounter();
			clock_Envelope();
			break;
		case 2:
			clock_Envelope();
			break;
		case 3:
			break;
		case 4:
			clock_Envelope();
			clock_LengthCounter();
			clock_Sweep();
			//wrap back to 0
			step = 0;
			return;
		}
	}

	step++;
}

void NesApu::clock_LengthCounter()
{
	if (!len_disable && !len_counter){
			len_counter--;
	}
}

void NesApu::clock_Envelope()
{
	if (write_to_fourth_channel)
	{
		write_to_fourth_channel = false;
		decay_hidden_vol = 0xF;
		decay_counter = vol;
	}
	else
	{
		if (decay_counter > 0)
			--decay_counter;
		else
		{
			decay_counter = vol;
			if (decay_hidden_vol > 0)
				--decay_hidden_vol;
			else if (!loop_disable)
				decay_hidden_vol = 0xF;
		}
	}
}

Sint16 NesApu::getSample()
{
	//result is between 0.0 and 1.0
	Sint16 pulse_out = (Sint16)(95.52 / ((8128.0 / (output_sample + 0)) + 100));
	//convert to signed 16 bit value
	pulse_out = (Sint16)std::floor(pulse_out * 65535);
	pulse_out ^= 0x8000;
	return pulse_out;
}


void NesApu::clock_Sweep()
{
	uint16_t period_calc = period;

	if (write_to_sweep_register)
	{
		sweep_stepper = sweep_period;
		write_to_sweep_register = false;
	}
	else if (sweep_stepper <= 0)
	{
		sweep_stepper = sweep_period;
		if (sweep_enable && !isSweepForcingSilence())
		{
			period_calc >>= sweep_shift;
			if (sweep_negate) { period -= (period_calc + 1); }
			else { period += period_calc; }
		}
	}
	else { sweep_stepper--; }
}

void NesApu::step_apu(int cycles)
{
	cpu_cycles += cycles;

	if (cycles % 2 == 0)
	{
		for (int i = 0; i < (cycles / 2); i++)
		{
			if (freq_counter > 0)
			{
				freq_counter--;
			}
			else
			{
				freq_counter = period;
				duty_counter = (duty_counter + 1) & 0x7;
			}

			if (!isSweepForcingSilence() && (len_counter != 0) && (duty_table[duty][duty_counter]))
			{
				if (!env_disable)
				{
					output_sample = decay_hidden_vol;
				}
				else
				{
					output_sample = vol;
				}
			}
			else
			{
				output_sample = 0;
			}
		}
	}

	//Frame Sequencer ticks at 240 hz or every 7457.5 CPU cycles
	//but since APU runs at half the speed of the cpu i guess it should be 74578/2 = 3728.5 APU cycles?
	if (cpu_cycles >= cycles_to_next_sequence) /*using cpu cycles*/
	{
		clock_FrameSequencer();
		cpu_cycles -= 7458;
		cycles_to_next_sequence += 7458;
		if ((mode && (cycles_to_next_sequence >= 37281)) || (!mode && (cycles_to_next_sequence >= 29828)))
		{
			cycles_to_next_sequence = 0;
		}
	}
}

bool NesApu::isSweepForcingSilence()
{
	if (period < 8){
		return true;
	}
	else if (!sweep_negate && (((period + (period >> sweep_shift)) >= 0x800))) {
		return true;
	}
	else { return false; }
}