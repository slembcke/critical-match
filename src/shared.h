#ifndef _SHARED_H
#define _SHARED_H

#include "pixler.h"

extern u8 i, ix, iy, idx;
#pragma zpsym("i");
#pragma zpsym("ix");
#pragma zpsym("iy");
#pragma zpsym("idx");

extern u8 joy0, joy1;
extern const u8 PALETTE[];

typedef struct {} GameState;

#define GRID_W 8
#define GRID_H 12
extern u8 GRID[];

#define grid_block_idx(x, y) (u8)(GRID_W*(y) + (x))

void grid_set_block(u8 x, u8 y, u8 block);
void grid_init(void);
void grid_update(void);

void player_init(void);
void player_tick(u8 joy);

GameState board(void);


GameState debug_chr(void);
void debug_hex(u8 value);

#endif
