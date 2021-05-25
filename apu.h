#ifndef APU_H
#define APU_H

#include "SDL_audio.h"
#include "SDL.h"
#include <iostream>
#include <cstdio>

class Bus;

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

class NesApu
{
private:

	Bus* busPtr;

	//
	double output_sample;

	int cpu_cycles;

	uint8_t step;
	uint8_t len_counter;
	uint8_t env_counter;
	uint8_t env_divider;
	uint8_t env_stepper;
	uint8_t sweep_divider;
	uint8_t sweep_stepper;
	bool halt;
	bool interrupt;
	bool write_to_fourth_channel;
	bool write_to_sweep_register;

	//General
	bool SQ1_enable;
	bool SQ2_enable;
	bool DMC_enable;
	bool Noise_enable;
	bool Triangle_enable;
	uint8_t apu_status;
	//

	//SQ1/SQ2
	bool mode, irq_disable;
	uint8_t duty, vol;
	bool loop_disable, env_disable;
	bool sweep_enable, negate;
	uint8_t sweep_period, shift;
	uint8_t length_index;
	uint16_t period;
	//

public:

	//Attaches APU to the NES
	void ConnectTotBus(Bus* ptr) {
		busPtr = ptr;
	}

	//Register read/writes
	uint8_t apu_read(uint16_t);
	void apu_write(uint16_t, uint8_t);

	//
	void clock_FrameSequencer();
	void clock_LengthCounter();
	void clock_Sweep();
	void clock_Envelope();
	//

	void apuStep(int);
};

#endif

