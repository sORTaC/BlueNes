#ifndef APU_H
#define APU_H

#include "SDL_audio.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <iostream>
#include <cstdio>

int dutytable[] = { 0b00000001, 0b00000011, 0b00001111, 0b11111100 };
int lenTable(uint8_t a) {
	switch (a) {
	case 0x00:return 10;
	case 0x02:return 20;
	case 0x04:return 40;
	case 0x06:return 80;
	case 0x0A:return 160;
	case 0x0C:return 60;
	case 0x0E:return 14;
	case 0x10:return 12;
	case 0x12:return 24;
	case 0x14:return 48;
	case 0x16:return 96;
	case 0x18:return 192;
	case 0x1A:return 72;
	case 0x1C:return 16;
	case 0x1E:return 32;
	case 0x1F:return 30;
	case 0x1D:return 28;
	case 0x1B:return 26;
	case 0x19:return 24;
	case 0x17:return 22;
	case 0x15:return 20;
	case 0x13:return 18;
	case 0x11:return 16;
	case 0x0F:return 14;
	case 0x0D:return 12;
	case 0x0B:return 10;
	case 0x09:return 8;
	case 0x07:return 6;
	case 0x05:return 4;
	case 0x03:return 2;
	case 0x01:return 254;
	}
}

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
public:
	void ConnectTotBus(Bus* ptr) {
		busPtr = ptr;
	}
	int ClocksToNextSequence(void);
	bool IsSweepForcingSilence();
	void Clock_QuarterFrame();
	void Clock_HalfFrame();
	void Init();
	void pulse1();
	uint8_t apu_read(uint16_t);
	void apu_write(uint16_t, uint8_t);
};

#endif

