#ifndef APU_H
#define APU_H

#include "SDL_audio.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <iostream>
#include <cstdio>

class Bus;

class NesApu
{
private:
	Bus* busPtr;
	int apu_cycles;
	int lenCounter, duty_counter, duty_table;
	int decay_loop, decay_V, decay_counter, decay_hidden_vol;
	int sweep_counter, sweep_timer, sweep_neg, sweep_shift, sweep_reload;
	int sequencer_mode, sequence_counter, next_seq_phase;
	int freq_counter, freq_timer;
	bool decay_reset_flag, decayEn;;
	bool irq_en, irq_pending, sweepEn, lenEn ,channelP1En;
	int output;
	double finalSample;
public:
	void ConnectTotBus(Bus* ptr) {
		busPtr = ptr;
	}
    void step_apu();
	int ClocksToNextSequence(void);
	bool IsSweepForcingSilence();
	void Clock_QuarterFrame();
	void Clock_HalfFrame();
	void Init();
	double getSample();
	void pulse1();
	uint8_t apu_read(uint16_t);
	void apu_write(uint16_t, uint8_t);
};

#endif

