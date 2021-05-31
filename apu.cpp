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
		vol = data & 0x4;
		env_disable = (data >> 4) & 0x1;
		loop_disable = (data >> 5) & 0x1;
		duty = (data >> 6) & 0x3;
		halt = (data >> 5) & 0x1;
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
		period |= ((data << 8) & 0x7);
		length_index = (data >> 3);
		//a write to the channel's 4th register immediately loads it with the value from len_table
		if (!loop_disable)
		{
			len_counter = len_table[(length_index >> 4)][(length_index >> 3) & 0x1];
		}
		duty_counter = 0;
		freq_counter = period;
		break;
	case 0x4015:
		SQ1_enable = (data >> 2) & 0x1;
		SQ2_enable = (data >> 3) & 0x1;
		Noise_enable = (data >> 5) & 0x1;
		Triangle_enable = (data >> 4) & 0x1;
		apu_status = data;
	case 0x4017:
		mode = data & 0x80;
		irq_disable = data & 0x40;
		if (mode)/*mode 1 is immediately clocked*/
		{
			clock_Sweep();
			clock_Envelope();
			clock_LengthCounter();
		}
		if (irq_disable)
		{
			interrupt = false;
		}
		break;
	default:
		printf("Addr here is not handled yet. Only working with sq1 right now =)");
		break;
	}
}

uint8_t NesApu::apu_read(uint16_t addr)
{
	if (addr == 0x4015)
	{
		uint8_t output = 0;
		if (len_counter != 0)
			output |= 0x1;
		if (interrupt)
			output |= 0x40;
		interrupt = false;
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

	if (freq_counter > 0)
	{
		freq_counter--;
	}
	else
	{
		freq_counter = period;
		duty_counter = (duty_counter + 1) & 0x7;
	}

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
				interrupt = true;
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

	/*
	A length counter allows automatic duration control. Counting can be halted and
	the counter can be disabled by clearing the appropriate bit in the status
	register, which immediately sets the counter to 0 and keeps it there.
	*/

	if (loop_disable)
	{
		len_counter = 0;
	}

	/*
	When clocked by the frame sequencer, if the halt flag is clear and the counter
	is non-zero, it is decremented.
	*/

	if (!loop_disable)

	{
		if ((!halt) && (!len_counter))
		{
			if (len_counter != 0)
				len_counter--;
		}
	}
}

void NesApu::clock_Envelope()
{
	/*
	When disable is set, the channel's volume is n, otherwise it is the value in
	the counter. Unless overridden by some other condition, the channel's DAC
	receives the channel's volume value.
	*/
	if (!isSweepForcingSilence() && (len_counter != 0) && (duty_table[duty][duty_counter]))
	{
		if (env_disable)
		{
			output_sample = vol;
		}
		else
		{
			output_sample = env_counter;
		}
	}

	/*
	The divider's period is set to n + 1.

	When clocked by the frame sequencer, one of two actions occurs: if there was a
	write to the fourth channel register since the last clock, the counter is set
	to 15 and the divider is reset, otherwise the divider is clocked.

	When the divider outputs a clock, one of two actions occurs: if loop is set and
	counter is zero, it is set to 15, otherwise if counter is non-zero, it is
	decremented.
	*/
	if (!env_disable)
	{
		if (write_to_fourth_channel)
		{
			env_counter = 15;
			env_divider = 0;
		}
		else
		{
			//clock divider

			if (env_stepper >= (vol + 1))
			{
				if (!loop_disable && !env_counter)
				{
					env_counter = 15;
				}
				else if (!env_counter)
				{
					env_counter--;
				}
				env_stepper -= (vol + 1);
			}
			env_stepper++;
		}
		write_to_fourth_channel = false;
	}
}


void NesApu::clock_Sweep()
{
	if (write_to_sweep_register)
	{
		sweep_stepper = sweep_period;
		write_to_sweep_register = false;
	}

	uint16_t period_calc = period;

	/*
	When the sweep unit is clocked, the divider is *first* clocked and then if
	there was a write to the sweep register since the last sweep clock, the divider
	is reset.
	*/

	if (sweep_stepper >= (sweep_period + 1))
	{
		if (sweep_enable && !isSweepForcingSilence())
		{
			period_calc <<= sweep_shift;

			if (sweep_negate)
			{
				period_calc = ~period;
			}

			period += period_calc;

			sweep_stepper -= (sweep_period + 1);
		}
	}
	else 
		sweep_stepper++;
}

void NesApu::apuStep(int cycles)
{
	cpu_cycles += cycles;

	//Frame Sequencer ticks at 240 hz or every 7457.5 CPU cycles
	//but since APU runs at half the speed of the cpu i guess it should be 74578/2 = 3728.5 APU cycles?
	if (cpu_cycles >= 7458) /*using cpu cycles*/
	{
		clock_FrameSequencer();
		cpu_cycles -= 7458;
	}
}

double NesApu::getSample()
{
	sample_updater++;
	return output_sample;
}

bool NesApu::isSweepForcingSilence()
{
	if (period < 8)
	{
		return true;
	}
	/*
When the channel's period is less than 8 or the result of the shifter is
greater than $7FF, the channel's DAC receives 0 and the sweep unit doesn't
change the channel's period.

Otherwise, if the sweep unit is enabled and the
shift count is greater than 0, when the divider outputs a clock, the channel's
period in the third and fourth registers are updated with the result of the
shifter.

*/
	else if (!sweep_negate && ((period + (period >> sweep_shift) >= 0x800)))
	{
		return true;
	}
	else
		return false;
}