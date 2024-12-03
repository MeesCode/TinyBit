#ifndef AUDIO_H
#define AUDIO_H

typedef enum {
	Ab = 8, Gs = 8,
	A = 9,
	As = 10, Bb = 10,
	B = 11, Cb = 11,
	C = 0, Bs = 0,
	Cs = 1, Db = 1,
	D = 2,
	Ds = 3, Eb = 3,
	E = 4, Fb = 4,
	F = 5, Es = 5,
	Fs = 6, Gb = 6,
	G = 7,
} TONE;

typedef enum {
	SINE,
	SAW,
	SQUARE,
	NOISE,
	REST
} WAVEFORM;

extern SDL_AudioDeviceID audio_device;
extern SDL_AudioSpec audio_spec;
extern int bpm;
extern int channel;
extern int volume;

void lua_setup_audio();
void audio_init();
void set_bpm(int);
void set_channel(int);
void set_volume(int);
void play_tone(TONE tone, int octave, int eights, WAVEFORM w, int, int);
void play_noise(int eights, int, int);

#endif