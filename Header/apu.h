#ifndef APU_H
#define APU_H

#include "SDL_audio.h"
#include "SDL.h"
#include <iostream>
#include <cstdio>

class Bus;

class NesApu
{
private:

	Bus* busPtr;

	Sint16 output_sample;
	int cpu_cycles;
	int cycles_to_next_sequence;

	uint8_t decay_hidden_vol;
	uint8_t decay_counter;
	uint8_t duty_counter;
	uint16_t freq_counter;
	uint8_t step;
	uint8_t len_counter;
	uint8_t env_counter;
	uint8_t env_divider;
	uint8_t env_stepper;
	uint8_t sweep_divider;
	uint8_t sweep_stepper;
	bool len_disable = false;
	bool irq_pending = false;
	bool write_to_fourth_channel = false;
	bool write_to_sweep_register = false;

	//General
	bool SQ1_enable = false;
	bool SQ2_enable = false;
	bool DMC_enable = false;
	bool Noise_enable = false;
	bool Triangle_enable = false;
	//

	//SQ1/SQ2
	bool mode = false;
	bool irq_disable = false;
	bool loop_disable = false;
	bool env_disable = false;
	bool sweep_enable = false;
	bool sweep_negate = false;
	uint8_t duty, vol;
	uint8_t sweep_period, sweep_shift;
	uint8_t length_index;
	uint16_t period;
	//

public:
	NesApu();

	//Attaches APU to the NES
	void ConnectTotBus(Bus* ptr) {
		busPtr = ptr;
	}

	//Register read/writes
	uint8_t apu_read(uint16_t);
	void apu_write(uint16_t, uint8_t);

	//
	bool isSweepForcingSilence();
	void clock_FrameSequencer();
	void clock_LengthCounter();
	void clock_Sweep();
	void clock_Envelope();
	Sint16 getSample();
	//

	void step_apu(int);
};

#endif

