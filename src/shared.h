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

GameState board(void);

GameState debug_chr(void);

#endif
