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

	//
	Sint16 output_sample;

	int sample_updater = 0;
	int cpu_cycles;

	uint8_t duty_counter;
	uint16_t freq_counter;
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
	bool sweep_enable, sweep_negate;
	uint8_t sweep_period, sweep_shift;
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
	int getSampleNumber() { return sample_updater; }
	void resetSampleNumber() { sample_updater = 0; }
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

