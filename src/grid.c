#include <stdlib.h>
#include <string.h>
#include <lz4.h>

#include "pixler/pixler.h"
#include "coro/coro.h"
#include "shared.h"

static const u8 DROPS[] = {
	0x33, 0x12, 0x20, 0x42, 0x17, 0x00, 0x01, 0x51, 0x70, 0x22, 0x03, 0x26, 0x23, 0x10, 0x04, 0x11, 0x31, 0x13, 0x35, 0x62, 0x31, 0x03, 0x22, 0x30,
	0x12, 0x13, 0x63, 0x23, 0x51, 0x11, 0x32, 0x12, 0x70, 0x21, 0x36, 0x00, 0x35, 0x22, 0x23, 0x00, 0x40, 0x14, 0x03, 0x27, 0x02, 0x01, 0x30, 0x31,
	0x50, 0x00, 0x20, 0x03, 0x34, 0x20, 0x11, 0x12, 0x71, 0x10, 0x33, 0x21, 0x03, 0x35, 0x27, 0x11, 0x22, 0x61, 0x36, 0x32, 0x02, 0x13, 0x43, 0x02,
	0x10, 0x16, 0x32, 0x11, 0x33, 0x02, 0x62, 0x13, 0x04, 0x41, 0x25, 0x20, 0x21, 0x70, 0x51, 0x23, 0x02, 0x33, 0x00, 0x37, 0x12, 0x31, 0x00, 0x23,
	0x30, 0x02, 0x22, 0x33, 0x62, 0x01, 0x25, 0x52, 0x00, 0x26, 0x10, 0x03, 0x10, 0x23, 0x71, 0x14, 0x12, 0x07, 0x33, 0x21, 0x41, 0x31, 0x10, 0x33,
	0x10, 0x37, 0x01, 0x23, 0x20, 0x13, 0x02, 0x10, 0x31, 0x52, 0x01, 0x02, 0x36, 0x25, 0x74, 0x63, 0x13, 0x22, 0x40, 0x32, 0x00, 0x11, 0x33, 0x21,
	0x10, 0x21, 0x21, 0x02, 0x42, 0x11, 0x25, 0x27, 0x10, 0x11, 0x03, 0x31, 0x63, 0x53, 0x00, 0x00, 0x30, 0x34, 0x72, 0x33, 0x36, 0x23, 0x12, 0x02,
	0x23, 0x20, 0x17, 0x03, 0x14, 0x05, 0x00, 0x31, 0x51, 0x33, 0x63, 0x41, 0x02, 0x11, 0x20, 0x32, 0x10, 0x12, 0x32, 0x32, 0x21, 0x06, 0x70, 0x23,
};

static const u8 DROP_BLOCKS[] = {
	BLOCK_CHEST | BLOCK_COLOR_BLUE,
	BLOCK_CHEST | BLOCK_COLOR_RED,
	BLOCK_CHEST | BLOCK_COLOR_GREEN,
	BLOCK_CHEST | BLOCK_COLOR_PURPLE,
	BLOCK_KEY | BLOCK_COLOR_BLUE,
	BLOCK_KEY | BLOCK_COLOR_RED,
	BLOCK_KEY | BLOCK_COLOR_GREEN,
	BLOCK_KEY | BLOCK_COLOR_PURPLE,
};

static u8 DROP_X[] = {
	4, 5, 6, 2, 3, 1,
	3, 2, 1, 5, 4, 6,
	1, 4, 2, 6, 5, 3,
	4, 1, 6, 5, 3, 2,
	4, 5, 2, 6, 1, 3,
	2, 6, 4, 1, 3, 5,
	5, 2, 6, 1, 3, 4,
	2, 3, 4, 5, 1, 6,
	6, 1, 3, 4, 2, 5,
	1, 3, 2, 4, 5, 6,
};

// Block values in the grid.
u8 GRID[GRID_BYTES];

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
	u8 drop_counter;
	u8 state_timer;
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
#define BLOCK_MATCH(block, cmp) (((block ^ cmp) & BLOCK_MATCH_MASK) == BLOCK_STATUS_MATCHING)

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
				BLOCK_MATCH(block, GRID_D[idx]) ||
				(COLUMN_HEIGHT[ix] > iy && BLOCK_MATCH(block, GRID_U[idx])) ||
				(COLUMN_HEIGHT_L[ix] >= iy && BLOCK_MATCH(block, GRID_L[idx])) ||
				(COLUMN_HEIGHT_R[ix] >= iy && BLOCK_MATCH(block, GRID_R[idx])) ||
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
				return true;
			}
		}
	}
	
	return false;
}

static void grid_tick(void){
	grid_update_column_height();
	
	// Make blocks fall.
	for(iy = 1; iy < GRID_H - 1; ++iy){
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, iy);
			
			if(GRID[idx] == BLOCK_EMPTY && GRID_U[idx] != BLOCK_EMPTY){
				GRID[idx] = GRID_U[idx];
				GRID_U[idx] = BLOCK_EMPTY;
			} else if(GRID[idx] & BLOCK_STATUS_UNLOCKED){
				// Remove unlocked blocks.
				GRID[idx] = BLOCK_EMPTY;
			}
		}
	}
	
	// TODO fail if column height prevents adding block?
	// Drop in new blocks if the field is clear.
	if(!grid_any_falling()){
		u8 drop = DROPS[grid.drop_counter];
		u8 block;
		
		ix = DROP_X[grid.drop_counter];
		
		// Push the first block directly onto the screen.
		idx = (drop >> 0) & 0x7;
		block = DROP_BLOCKS[idx];
		grid_set_block(grid_block_idx(ix, GRID_H - 2), block);
		
		// Write the second block into GRID and let it fall onto the screen.
		idx = (drop >> 4) & 0x7;
		block = DROP_BLOCKS[idx];
		idx = grid_block_idx(ix, GRID_H - 1);
		GRID[idx] = block;
		
		grid.drop_counter += 1;
		// TODO wrap drop counter.
	}
}

uintptr_t grid_update_coro(uintptr_t _){
	while(true){
		// Look for matches while waiting for the next tick.
		// TODO magic frame number.
		for(grid.state_timer = 0; grid.state_timer < 60; ++grid.state_timer){
			if(grid_open_chests()){
				// Prevent the timer from advancing as long as matches are happening.
				grid.state_timer = 0;
			}
			
			coro_yield(true);
		}
		
		grid_tick();
		coro_yield(true);
		
		// Blit the blocks to the screen over several frames.
		for(grid.state_timer = 1; grid.state_timer < GRID_H - 1; ++grid.state_timer){
			px_buffer_inc(PX_INC1);
			
			for(ix = 1; ix < GRID_W - 1; ++ix){
				idx = grid_block_idx(ix, grid.state_timer);
				grid_set_block(idx, GRID[idx]);
			}
			
			coro_yield(true);
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
	
	grid.drop_counter = 0;
	
	coro_start(grid_update_coro);
}

void grid_update(void){
	// TODO bind coroutine.
	coro_resume(0);
}
