#include <stdlib.h>
#include <string.h>
#include <lz4.h>

#include "pixler/pixler.h"
#include "shared.h"

// Block values in the grid.
u8 GRID[GRID_W*GRID_H];

// Height of each stacked column.
static u8 COLUMN_HEIGHT[GRID_W];

// Aliases for left/right/up/down from current block.
#define GRID_L (GRID + -1)
#define GRID_R (GRID +  1)
#define GRID_U (GRID +  GRID_W)
#define GRID_D (GRID + -GRID_W)

#define COLUMN_HEIGHT_L (COLUMN_HEIGHT + -1)
#define COLUMN_HEIGHT_R (COLUMN_HEIGHT +  1)

static const u16 ROW_ADDRS[] = {
	NT_ADDR(0, 8, 26 - 2* 0),
	NT_ADDR(0, 8, 26 - 2* 1),
	NT_ADDR(0, 8, 26 - 2* 2),
	NT_ADDR(0, 8, 26 - 2* 3),
	NT_ADDR(0, 8, 26 - 2* 4),
	NT_ADDR(0, 8, 26 - 2* 5),
	NT_ADDR(0, 8, 26 - 2* 6),
	NT_ADDR(0, 8, 26 - 2* 7),
	NT_ADDR(0, 8, 26 - 2* 8),
	NT_ADDR(0, 8, 26 - 2* 9),
	NT_ADDR(0, 8, 26 - 2*10),
	NT_ADDR(0, 8, 26 - 2*11),
};

void buffer_set_metatile(u8 index, u16 addr);

void grid_set_block(u8 index, u8 block){
	px_buffer_inc(PX_INC1);
	// TODO This generates garbage assembly.
	buffer_set_metatile(block, ROW_ADDRS[index >> 3] + (((index & 0x7) << 1)));
	
	// idx = grid_block_idx(x, y);
	GRID[index] = block;
}

static void grid_open_chests(void){
	static u8 queue[8];
	register u8 cursor = 0;
	
	register u8 block, color, type;
	
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = COLUMN_HEIGHT[ix]; iy > 0; --iy){
			idx = grid_block_idx(ix, iy);
			block = GRID[idx];
			color = block & BLOCK_MASK_COLOR;
			
			if((block & BLOCK_MASK_TYPE) != BLOCK_CHEST) continue;
			
			if(GRID_D[idx] == (BLOCK_KEY | color) || GRID_D[idx] == (BLOCK_OPEN | color)){
				queue[cursor] = idx;
				if(++cursor == sizeof(queue)) goto open_queued_chests;
			}
			
			if(COLUMN_HEIGHT[ix] >= iy + 1 && (GRID_U[idx] == (BLOCK_KEY | color) || GRID_U[idx] == (BLOCK_OPEN | color))){
				queue[cursor] = idx;
				if(++cursor == sizeof(queue)) goto open_queued_chests;
			}
			
			if(COLUMN_HEIGHT_R[ix] >= iy && (GRID_R[idx] == (BLOCK_KEY | color) || GRID_R[idx] == (BLOCK_OPEN | color))){
				queue[cursor] = idx;
				if(++cursor == sizeof(queue)) goto open_queued_chests;
			}
			
			if(COLUMN_HEIGHT_L[ix] >= iy && (GRID_L[idx] == (BLOCK_KEY | color) || GRID_L[idx] == (BLOCK_OPEN | color))){
				queue[cursor] = idx;
				if(++cursor == sizeof(queue)) goto open_queued_chests;
			}
		}
	}
	
	open_queued_chests:
	while(cursor > 0){
		--cursor;
		
		idx = queue[cursor];
		color = GRID[idx] & BLOCK_MASK_COLOR;
		grid_set_block(idx, BLOCK_OPEN | color);
	}
}

static void grid_tick(void){
	// Calculate column heights.
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = 1; iy < GRID_H - 1; ++iy){
			idx = grid_block_idx(ix, iy);
			if(GRID[idx] == 0) break;
		}
		
		--iy;
		COLUMN_HEIGHT[ix] = iy;
	}
	
	grid_open_chests();
}

void grid_init(void){
	static const u8 TEMPLATE[] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
	};
	
	// TODO could shorten this with some magic.
	memcpy(GRID, TEMPLATE, sizeof(GRID));
}

void grid_update(void){
	static u8 frames = 1;
	register u8 above;
	
	if(true){
		// Debug draw stack heights.
		idx = px_ticks & 0x7;
		ix = 68 + 16*idx;
		iy = 200 - 16*COLUMN_HEIGHT[idx];
		px_spr(ix, iy, 0x00, '*');
	}
	
	if(frames < GRID_H - 1){
		px_buffer_inc(PX_INC1);
		
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, frames);
			
			if(GRID[idx] == 0){
				above = GRID_U[idx];
				if(above != 0){
					// TODO split this across frames to avoid using so much buffer memory?
					grid_set_block(idx, above);
					grid_set_block(idx + GRID_W, 0);
				}
			}
		}
	}
	
	++frames;
	if(frames >= 32){
		frames = 1;
		grid_tick();
	}
}
