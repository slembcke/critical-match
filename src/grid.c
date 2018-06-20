#include <stdlib.h>
#include <string.h>

#include "pixler.h"
#include "shared.h"

u8 GRID[GRID_W*GRID_H] = {};

// #define PX_WRITE_IDX_TO_BUFFER(i) {asm("lda %v", idx); asm("ldy #%b", i); asm("sta (%v + %b), y", PX, offsetof(PX_t, buffer));}

void grid_set_block(u8 x, u8 y, u8 block){
	px_buffer_inc(PX_INC1);
	px_buffer_set_metatile(block, NT_ADDR(0, 8 + 2*x, 26 - 2*y));
	
	idx = grid_block_idx(x, y);
	GRID[idx] = block;
}

static u8 GRID_HEIGHT[GRID_W] = {};
static void grid_tick(void){
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = 1; iy < GRID_H - 1; ++iy){
			idx = grid_block_idx(ix, iy);
			if(GRID[idx] == 0) break;
		}
		
		GRID_HEIGHT[ix] = iy;
	}
	
	// px_buffer_inc(PX_INC1);
	// px_buffer_data(6, NT_ADDR(0, 0, 1));
	// PX.buffer[0] = '0' - 1 + GRID_HEIGHT[1];
	// PX.buffer[1] = '0' - 1 + GRID_HEIGHT[2];
	// PX.buffer[2] = '0' - 1 + GRID_HEIGHT[3];
	// PX.buffer[3] = '0' - 1 + GRID_HEIGHT[4];
	// PX.buffer[4] = '0' - 1 + GRID_HEIGHT[5];
	// PX.buffer[5] = '0' - 1 + GRID_HEIGHT[6];
}

// TODO Is this code bigger than a table? LOL
void grid_init(void){
	static const u8 row[] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
	
	memset(GRID, 0xFF, 1*GRID_W);
	memcpy(GRID +  1*GRID_W, row, GRID_W);
	memcpy(GRID +  2*GRID_W, row, GRID_W);
	memcpy(GRID +  3*GRID_W, row, GRID_W);
	memcpy(GRID +  4*GRID_W, row, GRID_W);
	memcpy(GRID +  5*GRID_W, row, GRID_W);
	memcpy(GRID +  6*GRID_W, row, GRID_W);
	memcpy(GRID +  7*GRID_W, row, GRID_W);
	memcpy(GRID +  8*GRID_W, row, GRID_W);
	memcpy(GRID +  9*GRID_W, row, GRID_W);
	memcpy(GRID + 10*GRID_W, row, GRID_W);
	memcpy(GRID + 11*GRID_W, row, GRID_W);
}

void grid_update(void){
	static u8 frames = 1;
	u8 above;
	
	if(frames < GRID_H - 1){
		px_buffer_inc(PX_INC1);
		
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, frames + 1);
			above = GRID[idx];
			
			idx -= GRID_W;
			if(GRID[idx] == 0 && above != 0){
				// TODO split this across frames to avoid using so much buffer memory?
				grid_set_block(ix, frames, above);
				grid_set_block(ix, frames + 1, 0);
			}
		}
	}
	
	++frames;
	if(frames >= 32){
		frames = 1;
		grid_tick();
	}
}
