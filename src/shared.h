#ifndef _SHARED_H
#define _SHARED_H

#include "pixler/pixler.h"

extern u8 i, ix, iy, idx;
#pragma zpsym("i");
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
#define BLOCK_MASK_COLOR 0x03
#define BLOCK_BLUE 0x00
#define BLOCK_RED 0x01
#define BLOCK_GREEN 0x02
#define BLOCK_PURPLE 0x03

// Block type.
#define BLOCK_MASK_TYPE 0x0C
#define BLOCK_EMTPY 0x00
#define BLOCK_CHEST 0x04
#define BLOCK_KEY 0x08
#define BLOCK_OPEN 0x0C

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


GameState debug_chr(void);
GameState debug_freeze(void);
void debug_hex(u8 value);

#endif
