#ifndef _SHARED_H
#define _SHARED_H

#include "pixler/pixler.h"

extern u8 ix, iy, idx;
#pragma zpsym("ix");
#pragma zpsym("iy");
#pragma zpsym("idx");

typedef struct {} AudioChunk;
extern const AudioChunk MUSIC[];
extern const AudioChunk SOUNDS[];

void music_init(const AudioChunk *music);
void sound_init(const AudioChunk *sounds);
void music_play(u8 song);
void music_pause();
void music_stop();

enum {
	// Magic numbers from Famitone.
	// Is there a better way to expose these?
	SOUND_CH0 = (0*15) << 8,
	SOUND_CH1 = (1*15) << 8,
	SOUND_CH2 = (2*15) << 8,
	SOUND_CH3 = (3*15) << 8,
};

void sound_play(u16 sound);

typedef struct {} GameState;

#ifdef DEBUG
	#define DEBUG_PROFILE_START() px_profile_start()
	#define DEBUG_PROFILE_END() px_profile_end()
#else
	#define DEBUG_PROFILE_START()
	#define DEBUG_PROFILE_END()
#endif

#endif
