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

typedef struct {
	u8 drop_queue[2];
	u8 drop_x;
} Grid;

static Grid grid;

void buffer_set_metatile(u8 index, u16 addr);

void grid_set_block(u8 index, u8 block){
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
	
	register u16 addr;
	
	// addr = ROW_ADDRS[index >> 3];
	asm("ldy #%o", index); \
	asm("lda (sp), y"); \
	asm("tay"); \
	asm("lsr a"); \
	asm("lsr a"); \
	asm("and #$FE"); \
	asm("tax"); \
	asm("lda %v+0, x", ROW_ADDRS); \
	asm("sta %v+0", addr); \
	asm("lda %v+1, x", ROW_ADDRS); \
	asm("sta %v+1", addr);
	
	// addr += (((index & 0x7) << 1));
	asm("tya"); \
	asm("and #$07"); \
	asm("asl a"); \
	asm("clc"); \
	asm("adc %v+0", addr); \
	asm("sta %v+0", addr);
	
	px_buffer_inc(PX_INC1);
	buffer_set_metatile(block & ~BLOCK_STATUS_MASK, addr);
	
	GRID[index] = block;
}

// Only care about color and the 'matching' bit.
#define BLOCK_MATCH_MASK (BLOCK_COLOR_MASK | BLOCK_STATUS_MATCHABLE | BLOCK_STATUS_MATCHING)

// XOR a block with a neighbor.
// The color should be equal (zeroed bits), and the 'matching' bit should not.
// Ignoring other bits with a mask should leave only the 'matching' bit.
#define BLOCK_MATCH(block, arr, idx) (((block ^ arr[idx]) & BLOCK_MATCH_MASK) == BLOCK_STATUS_MATCHING)

static bool grid_open_chests(void){
	static u8 queue[8];
	register u8 cursor = 0;
	register u8 block;
	
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = COLUMN_HEIGHT[ix]; iy > 0; --iy){
			idx = grid_block_idx(ix, iy);
			block = GRID[idx];
			
			if(
				// Skip blocks that are already unlocked.
				(block & BLOCK_STATUS_UNLOCKED) ||
				// Skip blocks that are unmatchable (empty, border, etc)
				(block & BLOCK_STATUS_MATCHABLE) == 0
			) continue;
			
			if(
				BLOCK_MATCH(block, GRID_D, idx) ||
				(COLUMN_HEIGHT[ix] > iy && BLOCK_MATCH(block, GRID_U, idx)) ||
				(COLUMN_HEIGHT_L[ix] >= iy && BLOCK_MATCH(block, GRID_L, idx)) ||
				(COLUMN_HEIGHT_R[ix] >= iy && BLOCK_MATCH(block, GRID_R, idx)) ||
				false
			){
				queue[cursor] = idx;
				++cursor;
				
				// Ran out of queue space.
				if(cursor == sizeof(queue)) goto open_queued_chests;
			}
		}
	}
	
	if(cursor == 0) return false;
	
	open_queued_chests:
	while(cursor > 0){
		--cursor;
		idx = queue[cursor];
		block = GRID[idx];
		
		if((block & BLOCK_TYPE_MASK) == BLOCK_TYPE_CHEST){
			// Change chests into open chests.
			block ^= BLOCK_TYPE_CHEST ^ BLOCK_TYPE_OPEN;
		}
		
		grid_set_block(idx, block | BLOCK_STATUS_MATCHING | BLOCK_STATUS_UNLOCKED);
	}
	
	return true;
}

static void grid_fall(u8 row){
	px_buffer_inc(PX_INC1);
	
	if(row == 10){
		// Shift in queued blocks;
		grid_set_block(grid_block_idx(grid.drop_x, 10), grid.drop_queue[0]);
		grid.drop_queue[0] = grid.drop_queue[1];
		grid.drop_queue[1] = BLOCK_EMPTY;
	} else {
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, row);
			
			if(GRID[idx] == BLOCK_EMPTY && GRID_U[idx] != BLOCK_EMPTY){
				// TODO split this across frames to avoid using so much buffer memory?
				grid_set_block(idx, GRID_U[idx]);
				grid_set_block(idx + GRID_W, 0);
			} else if(GRID[idx] & BLOCK_STATUS_UNLOCKED){
				// Remove unlocked blocks.
				grid_set_block(idx, 0);
			}
		}
	}
}

static void grid_update_column_height(void){
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = 1; iy < GRID_H - 1; ++iy){
			idx = grid_block_idx(ix, iy);
			if(GRID[idx] == BLOCK_EMPTY) break;
		}
		
		--iy;
		COLUMN_HEIGHT[ix] = iy;
	}
}

static bool grid_any_falling(void){
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = COLUMN_HEIGHT[ix] + 1; iy < GRID_H - 1; ++iy){
			idx = grid_block_idx(ix, iy);
			if(GRID[idx] != BLOCK_EMPTY){
				debug_hex(idx);
				return true;
			}
		}
	}
	
	return false;
}

void grid_init(void){
	static const u8 ROW[] = {BLOCK_BORDER, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_BORDER};
	
	// Abuse memcpy to smear the row template.
	memcpy(GRID + 0x58, ROW, sizeof(ROW));
	memcpy(GRID + 0x08, GRID + 0x10, 0x50);
	memset(GRID, BLOCK_BORDER, 8);
	
	grid.drop_queue[0] = BLOCK_EMPTY;
	grid.drop_queue[1] = BLOCK_EMPTY;
	grid.drop_x = 1;
}

void grid_update(void){
	// TODO redo the timer logic.
	static u8 tick_timer = 1;
	
	#ifdef DEBUG
		// Debug draw stack heights.
		idx = px_ticks & 0x7;
		ix = 68 + 16*idx;
		iy = 200 - 16*COLUMN_HEIGHT[idx];
		px_spr(ix, iy, 0x00, '*');
	#endif
	
	if(tick_timer < GRID_H - 1){
		// Move blocks down for the first few frames.
		grid_fall(tick_timer);
	} else {
		// Then start looking for matches.
		if(grid_open_chests()){
			// Prevent the timer from advancing as long as matches are happening.
			tick_timer = GRID_H;
		}
	}
	
	++tick_timer;
	if(tick_timer >= 64){
		grid_update_column_height();
		
		if(!grid_any_falling()){
			grid.drop_queue[0] = BLOCK_CHEST | BLOCK_COLOR_GREEN;
			grid.drop_queue[1] = BLOCK_CHEST | BLOCK_COLOR_BLUE;
			grid.drop_x = 2;
		}
		
		tick_timer = 1;
	}
}
