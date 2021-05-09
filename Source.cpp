#include "apu.h"
#include "cpu.h"
#include "Bus.h"
#include "ppu.h"
#include "mem.h"
#include <queue>
#undef main

#define NB_SAMPLES 4096

int main()
{
	Bus main;
	cpu6502 cpu;
	NesPPU ppu;
	NesApu apu;
	main.ConnectBus(&cpu, &ppu, &apu);
	ppu.ConnectToBus(&main);
	cpu.ConnectTotBus(&main);
	apu.ConnectTotBus(&main);
	main.BusMapperSet();
	main.init();
	main.run();
	return 0;
}

////const int AMPLITUDE = 28000;
////const int FREQUENCY = 44100;
////
////struct BeepObject
////{
////	double freq;
////	int samplesLeft;
////};
////
////class Beeper
////{
////private:
////	double v;
////	SDL_AudioDeviceID dev;
////	std::queue<BeepObject>beeps;
////public:
////	Beeper();
////	void beep(double freq, int duration);
////	void generateSamples(Sint16* stream, int length);
////	void wait();
////};
////
////void audio_callback(void*, Uint8*, int);
////
////Beeper::Beeper()
////{
////    SDL_AudioSpec desiredSpec;
////
////    desiredSpec.freq = FREQUENCY;
////    desiredSpec.format = AUDIO_S16SYS;
////    desiredSpec.channels = 1;
////    desiredSpec.samples = 4096;
////    desiredSpec.callback = audio_callback;
////    desiredSpec.userdata = this;
////
////    SDL_AudioSpec obtainedSpec;
////
////    // you might want to look for errors here
////    dev = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &obtainedSpec, 0);
////
////    // start play audio
////    SDL_PauseAudioDevice(dev, 0);
////}
////
////void Beeper::generateSamples(Sint16* stream, int length)
////{
////    int i = 0;
////    while (i < length) {
////
////        if (beeps.empty()) {
////            while (i < length) {
////                stream[i] = 0;
////                i++;
////            }
////            return;
////        }
////        BeepObject& bo = beeps.front();
////
////        int samplesToDo = std::min(i + bo.samplesLeft, length);
////        bo.samplesLeft -= samplesToDo - i;
////
////        while (i < samplesToDo) {
////            stream[i] = AMPLITUDE * std::sin(v * 2 * M_PI / FREQUENCY);
////            i++;
////            v += bo.freq;
////        }
////
////        if (bo.samplesLeft == 0) {
////            beeps.pop();
////        }
////    }
////}
////
////void Beeper::beep(double freq, int duration)
////{
////    BeepObject bo;
////    bo.freq = freq;
////    bo.samplesLeft = duration * FREQUENCY / 1000;
////
////    SDL_LockAudio();
////    beeps.push(bo);
////    SDL_UnlockAudio();
////}
////
////void Beeper::wait()
////{
////    int size;
////    do {
////        SDL_Delay(20);
////        SDL_LockAudio();
////        size = beeps.size();
////        SDL_UnlockAudio();
////    } while (size > 0);
////}
////
////void audio_callback(void* _beeper, Uint8* _stream, int _length)
////{
////    Sint16* stream = (Sint16*)_stream;
////    int length = _length / 2;
////    Beeper* beeper = (Beeper*)_beeper;
////
////    beeper->generateSamples(stream, length);
////}
////
////int main(int argc, char* argv[])
////{
////    SDL_Init(SDL_INIT_AUDIO);
////
////    int duration = 1000;
////    double Hz = 440;
////
////    Beeper b;
////    b.beep(Hz, duration);
////    b.wait();
////
////    return 0;
////}
//
//#include <math.h>
//#include <SDL.h>
//#include <SDL_audio.h>
//
//const int AMPLITUDE = 28000;
//const int SAMPLE_RATE = 44100;
//
//void audio_callback(void* user_data, Uint8* raw_buffer, int bytes)
//{
//    Sint16* buffer = (Sint16*)raw_buffer;
//    int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
//    int& sample_nr(*(int*)user_data);
//
//    for (int i = 0; i < length; i++, sample_nr++)
//    {
//        double time = (double)sample_nr / (double)SAMPLE_RATE;
//        double output = (Sint16)(AMPLITUDE * sin(2.0f * M_PI * 440.0f * time)); // render 441 HZ sine wave
//        if (output > 0.0)
//            buffer[i] = AMPLITUDE;
//        else if (output < 0.0)
//            buffer[i] = -AMPLITUDE;
//        else
//            buffer[i] = 0.0;
//    }
//}
//
//int main(int argc, char* argv[])
//{
//    if (SDL_Init(SDL_INIT_AUDIO) != 0) SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
//
//    int sample_nr = 0;
//
//    SDL_AudioSpec want;
//    want.freq = SAMPLE_RATE; // number of samples per second
//    want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
//    want.channels = 1; // only one channel
//    want.samples = 2048; // buffer-size
//    want.callback = audio_callback; // function SDL calls periodically to refill the buffer
//    want.userdata = &sample_nr; // counter, keeping track of current sample number
//    SDL_AudioDeviceID dev;
//    SDL_AudioSpec have;
//    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
//    if (want.format != have.format) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to get the desired AudioSpec");
//
//    SDL_PauseAudioDevice(dev, 0); // start playing sound
//    SDL_Delay(1000); // wait while sound is playing
//    SDL_PauseAudioDevice(dev, 1); // stop playing sound
//
//    SDL_CloseAudio();
//
//    return 0;
//}

//#include <stdio.h>
//#include <math.h>
//#include <SDL.h>
//
//static double x = 0;
//
//float square(float x) {
//    return sin(x) > 0 ? 1 : -1;
//}
//
//static void my_callback(void* userdata, Uint8* stream, int len) {
//    Sint16* stream16 = (Sint16*)stream;
//
//    int nb_samples = len / sizeof(Sint16);
//    for (int i = 0; i < nb_samples; i++) {
//        x += .05f;
//        stream16[i] = sin(x) * 5000;
//        stream16[i] += square(x) * 1600;
//    }
//}
//
//int main(void) {
//    SDL_Init(SDL_INIT_AUDIO);
//
//    // the representation of our audio device:
//    SDL_AudioDeviceID audio_device;
//
//    // setting up our audio format:
//    SDL_AudioSpec audio_spec = { 0 };
//    audio_spec.freq = 44100; // sampling rate
//    audio_spec.format = AUDIO_S16SYS; // sample format
//    audio_spec.channels = 1; // number of channels
//    audio_spec.samples = 4096; // buffer size
//    audio_spec.callback = my_callback;
//    audio_spec.userdata = NULL;
//
//    // opening the default audio device:
//    audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
//
//    // unpausing the audio device: starts playing the queued data
//    SDL_PauseAudioDevice(audio_device, 0);
//
//    SDL_Delay(10 * 1000);
//
//    SDL_CloseAudioDevice(audio_device);
//    SDL_Quit();
//
//    return 0;
//}

