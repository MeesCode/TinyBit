#ifndef MUSIC_FUNCTIONS_H
#define MUSIC_FUNCTIONS_H

#include <SDL.h>

typedef enum {
	C = 0,
	Cs = 1, Db = 1,
	D = 2,
	Ds = 3, Eb = 3,
	E = 4,
	F = 5,
	Fs = 6, Gb = 6,
	G = 7, Gs = 7,
	Ab = 8,
	A = 9,
	As = 10, Bb = 10,
	B = 11,
} TONE;

typedef enum {
	SIN = 0
} WAVEFORM;

extern SDL_AudioDeviceID audio_device;
extern SDL_AudioSpec audio_spec;
extern int bpm;

void set_bpm(int);
void play_tone(TONE tone, int octave, int eights, WAVEFORM w);

#endif