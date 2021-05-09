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
	struct sequencer
	{
		uint32_t sequence = 0x0000;
		uint16_t timer = 0x0000;
		uint16_t reload = 0x0000;
		uint8_t output = 0x0000;
	};
	bool enQuarter;
	bool enHalf;
	int clock_count;
	int frame_clock_counter;
	bool pulse1_enable = false;
	double pulse1_sample = 0.0;
	sequencer pulse1_seq;
public:
	void ConnectTotBus(Bus* ptr) {
		busPtr = ptr;
	}
	uint8_t apu_read(uint16_t);
	void apu_write(uint16_t, uint8_t);
	uint8_t clock_pulse1(bool);
        void step_apu();
	double getSample();
	void pulse1();
};

#endif

