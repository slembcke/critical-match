#ifndef _SHARED_H
#define _SHARED_H

#include "pixler/pixler.h"

extern u8 ix, iy, idx;
#pragma zpsym("ix");
#pragma zpsym("iy");
#pragma zpsym("idx");

typedef struct {} Music;
extern const Music MUSIC[];

void music_init(const Music *music);
void music_play(u8 song);
void music_pause();
void music_stop();

extern u8 joy0, joy1;
extern const u8 PALETTE[];

typedef struct {} GameState;

// Block color.
#define BLOCK_COLOR_MASK 0x03
#define BLOCK_COLOR_BLUE 0x00
#define BLOCK_COLOR_RED 0x01
#define BLOCK_COLOR_GREEN 0x02
#define BLOCK_COLOR_PURPLE 0x03

// Block type.
#define BLOCK_TYPE_MASK 0x0C
#define BLOCK_TYPE_OTHER 0x00
#define BLOCK_TYPE_CHEST 0x04
#define BLOCK_TYPE_KEY 0x08
#define BLOCK_TYPE_OPEN 0x0C

// Block status bits.
#define BLOCK_STATUS_MASK 0xF0
#define BLOCK_STATUS_MATCHABLE 0x10
#define BLOCK_STATUS_MATCHING 0x20
#define BLOCK_STATUS_UNLOCKED 0x40

// Misc block defs.
#define BLOCK_EMPTY 0x00
#define BLOCK_BORDER 0x01
#define BLOCK_GARBAGE 0x02
#define BLOCK_CHEST (BLOCK_TYPE_CHEST | BLOCK_STATUS_MATCHABLE)
#define BLOCK_KEY (BLOCK_TYPE_KEY | BLOCK_STATUS_MATCHABLE | BLOCK_STATUS_MATCHING)
#define BLOCK_OPEN (BLOCK_TYPE_OPEN | BLOCK_STATUS_MATCHABLE | BLOCK_STATUS_MATCHING)

#define GRID_W 8
#define GRID_H 12
extern u8 GRID[];

#define grid_block_idx(x, y) (u8)(GRID_W*(y) | (x))

void grid_set_block(u8 index, u8 block);
void grid_init(void);
void grid_update(void);

void player_init(void);
void player_tick(u8 joy);

GameState board(void);

#ifdef DEBUG
	#define DEBUG_PROFILE_START() px_profile_start()
	#define DEBUG_PROFILE_END() px_profile_end()
#else
	#define DEBUG_PROFILE_START()
	#define DEBUG_PROFILE_END()
#endif

GameState debug_chr(void);
GameState debug_freeze(void);
void debug_hex(u8 value);

#endif
