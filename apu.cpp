#include "apu.h"
#include "Bus.h"

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
		shift = data & 0x7;
		negate = (data >> 3) & 0x1;
		sweep_period = (data >> 4) & 0x7;
		sweep_enable = (data >> 7) & 0x1;
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
		break;
	case 0x4015:
		SQ1_enable = (data >> 2) & 0x1;
		SQ2_enable = (data >> 3) & 0x1;
		Noise_enable = (data >> 5) & 0x1;
		Triangle_enable = (data >> 4) & 0x1;
		uint8_t apu_status;
	case 0x4017:
		mode = data & 0x80;
		irq_disable = data & 0x40;
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
		return apu_status;
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
			interrupt = true;
			clock_Sweep();
			clock_Envelope();
			clock_LengthCounter();
			//wrap back to 0
			step = 0;
			return;
		}
	}
	else if(mode)
	{
		//5-step sequence
		/*
			Clocked immediately when mode 5
		*/
		switch (step)
		{
		case 0:
			clock_Sweep();
			clock_Envelope();
			clock_LengthCounter();
			break;
		case 1:
			clock_Envelope();
			break;
		case 2:
			clock_Sweep();
			clock_Envelope();
			clock_LengthCounter();
			break;
		case 3:
			clock_Envelope();
			break;
		case 4:
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
	if (env_disable)
	{
		output_sample = vol;
	}
	else
	{
		env_counter = vol;
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

			if (env_stepper == (vol + 1))
			{
				if (!loop_disable && !env_counter)
				{
					env_counter = 15;
				}
				else if (!env_counter)
				{
					env_counter--;
				}
				env_stepper = 0;
			}
			env_stepper++;
		}
		write_to_fourth_channel = false;
	}
}


void NesApu::clock_Sweep()
{
	if (sweep_enable)
	{
		uint16_t period_calc = period;

		/*
		When the sweep unit is clocked, the divider is *first* clocked and then if
		there was a write to the sweep register since the last sweep clock, the divider
		is reset.
		*/

		if (sweep_stepper == (sweep_period + 1))
		{
			if (shift != 0)
			{
				period_calc <<= shift;

				if (negate)
				{
					period_calc = ~period;
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

				if ((period < 8) || (period_calc > 0x7ff))
				{
					output_sample = 0.0;
				}
				else
				{
					period += period_calc;
				}
			}
		}

		sweep_stepper++;

		if (write_to_sweep_register)
		{
			sweep_stepper = 0;
			write_to_sweep_register = false;
		}
	}
}

void NesApu::apuStep(int cycles)
{	
	cpu_cycles += cycles;

	//Frame Sequencer ticks at 240 hz or every 7457.5 CPU cycles

	if (cpu_cycles == 7458)
	{
		clock_FrameSequencer();
		cpu_cycles = 0;
	}
}