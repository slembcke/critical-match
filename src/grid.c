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

#define MATCH_KEY (BLOCK_CHEST ^ BLOCK_KEY)
#define MATCH_OPEN (BLOCK_CHEST ^ BLOCK_OPEN)

static void grid_open_chests(void){
	static u8 queue[8];
	register u8 cursor = 0;
	register u8 block, cmp;
	
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = COLUMN_HEIGHT[ix]; iy > 0; --iy){
			idx = grid_block_idx(ix, iy);
			block = GRID[idx];
			
			if((block & BLOCK_MASK_TYPE) != BLOCK_CHEST) continue;
			
			cmp = block ^ GRID_D[idx];
			if(cmp == MATCH_KEY || cmp == MATCH_OPEN) goto enqueue;
			
			cmp = block ^ GRID_U[idx];
			if(COLUMN_HEIGHT[ix] > iy && (cmp == MATCH_KEY || cmp == MATCH_OPEN)) goto enqueue;
			
			cmp = block ^ GRID_L[idx];
			if(COLUMN_HEIGHT_L[ix] >= iy && (cmp == MATCH_KEY || cmp == MATCH_OPEN)) goto enqueue;
			
			cmp = block ^ GRID_R[idx];
			if(COLUMN_HEIGHT_R[ix] >= iy && (cmp == MATCH_KEY || cmp == MATCH_OPEN)) goto enqueue;
			
			enqueue:{
				queue[cursor] = idx;
				++cursor;
				
				// Ran out of queue space.
				if(cursor == sizeof(queue)) goto open_queued_chests;
			}
		}
	}
	
	open_queued_chests:
	while(cursor > 0){
		--cursor;
		idx = queue[cursor];
		grid_set_block(idx, BLOCK_OPEN | (GRID[idx] & BLOCK_MASK_COLOR));
	}
}

static void grid_fall(u8 tick_timer){
	px_buffer_inc(PX_INC1);
	
	for(ix = 1; ix < GRID_W - 1; ++ix){
		idx = grid_block_idx(ix, tick_timer);
		
		if(GRID[idx] == 0){
			if(GRID_U[idx] != 0){
				// TODO split this across frames to avoid using so much buffer memory?
				grid_set_block(idx, GRID_U[idx]);
				grid_set_block(idx + GRID_W, 0);
			}
		}
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
}

void grid_init(void){
	static const u8 ROW[] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
	
	// Abuse memcpy to smear the row template.
	memcpy(GRID + 0x58, ROW, sizeof(ROW));
	memcpy(GRID + 0x08, GRID + 0x10, 0x50);
	memset(GRID, 0xFF, 8);
}

void grid_update(void){
	static u8 tick_timer = 1;
	
	if(true){
		// Debug draw stack heights.
		idx = px_ticks & 0x7;
		ix = 68 + 16*idx;
		iy = 200 - 16*COLUMN_HEIGHT[idx];
		px_spr(ix, iy, 0x00, '*');
	}
	
	if(tick_timer < GRID_H - 1){
		// Move blocks down for the first few frames.
		grid_fall(tick_timer);
	} else {
		// Then start looking for matches.
		grid_open_chests();
	}
	
	++tick_timer;
	if(tick_timer >= 32){
		grid_tick();
		tick_timer = 1;
	}
}
