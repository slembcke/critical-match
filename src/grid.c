#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lz4.h>

#include "pixler/pixler.h"
#include "naco/naco.h"
#include "shared.h"

// Block values in the grid.
u8 GRID[GRID_BYTES];

// Height of each stacked column.
u8 COLUMN_HEIGHT[GRID_W];

#define MAX_FALL_TICKS 60
#define MIN_FALL_TICKS 15
#define FALL_TICKS_DEC 5
#define DROPS_PER_SPEEDUP 16
#define COMBO_TIMEOUT 8
#define MAX_COMBO 5

typedef struct {
	u8 drop_cursor;
	u8 column_cursor;
	
	u8 garbage_cursor;
	u8 garbage_mask;
	
	u8 speedup_counter;
	u8 block_fall_ticks;
	
	u8 combo;
	u8 combo_ticks;
	u16 score;
	
	u8 state_timer;
	u8 update_coro[16];
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
			
			// Emit coins.
			coins_add_at(idx);
		}
		
		grid_set_block(idx, block | BLOCK_STATUS_MATCHING | BLOCK_STATUS_UNLOCKED);
	}
	
	return true;
}

void grid_update_column_height(void){
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

// TODO rewrite in asm?
static u8 lru_shuffle(register u8 *arr, u8 size, u8 mask, register u8 *cursor){
	u8 value;
	u8 idx = (*cursor) + (rand8() & mask);
	if(idx >= size) idx -= size;
	
	value = arr[idx];
	arr[idx] = arr[*cursor];
	arr[*cursor] = value;
	
	++(*cursor);
	if(*cursor >= size) *cursor = 0;
	
	return value;
}

static u8 get_shuffled_block(void){
	static u8 DROPS[] = {
		0, 1, 2, 3,
		0, 1, 2, 3,
		0, 1, 2, 3,
		0, 1, 2, 3,
		4, 5, 6, 7,
		0, 1, 2, 3,
		0, 1, 2, 3,
		0, 1, 2, 3,
		0, 1, 2, 3,
		4, 5, 6, 7,
	};

	static const u8 BLOCKS[] = {
		BLOCK_CHEST | BLOCK_COLOR_BLUE,
		BLOCK_CHEST | BLOCK_COLOR_RED,
		BLOCK_CHEST | BLOCK_COLOR_GREEN,
		BLOCK_CHEST | BLOCK_COLOR_PURPLE,
		BLOCK_KEY | BLOCK_COLOR_BLUE,
		BLOCK_KEY | BLOCK_COLOR_RED,
		BLOCK_KEY | BLOCK_COLOR_GREEN,
		BLOCK_KEY | BLOCK_COLOR_PURPLE,
	};
	
	return BLOCKS[lru_shuffle(DROPS, sizeof(DROPS), 0x7, &grid.drop_cursor)];
}

static u8 get_shuffled_column(void){
	static u8 COLUMNS[] = {1, 2, 3, 4, 5, 6};
	return lru_shuffle(COLUMNS, sizeof(COLUMNS), 0x3, &grid.column_cursor);
}

static void grid_drop_block(void){
	u8 block;
	
	ix = get_shuffled_column();
	idx = grid_block_idx(ix, GRID_H - 2);
	
	// Game over if the drop location isn't clear.
	if(GRID[idx] != BLOCK_EMPTY) game_over();
	
	// Push the first block directly onto the screen.
	block = get_shuffled_block();
	grid_set_block(idx, block);
	
	// Write the second block into GRID and let it fall onto the screen.
	block = get_shuffled_block();
	idx = grid_block_idx(ix, GRID_H - 1);
	GRID[idx] = block;
}

static void grid_update_fall_speed(void){
	--grid.speedup_counter;
	if(grid.speedup_counter == 0){
		grid.block_fall_ticks -= FALL_TICKS_DEC;
		if(grid.block_fall_ticks < MIN_FALL_TICKS){
			grid.block_fall_ticks = MIN_FALL_TICKS;
		}
		
		grid.speedup_counter = DROPS_PER_SPEEDUP;
	}
}

static void grid_blocks_tick(void){
	register u8 matched_blocks = 0;
	
	for(iy = 1; iy < GRID_H - 1; ++iy){
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, iy);
			
			if(GRID[idx] == BLOCK_EMPTY && GRID_U[idx] != BLOCK_EMPTY){
				// Block is unsupported, make it fall.
				GRID[idx] = GRID_U[idx];
				GRID_U[idx] = BLOCK_EMPTY;
			} else if(GRID[idx] & BLOCK_STATUS_UNLOCKED){
				// Match out unlocked blocks.
				if(GRID[idx] & BLOCK_TYPE_CHEST) ++matched_blocks;
				GRID[idx] = BLOCK_EMPTY;
			}
		}
	}
	
	grid_update_column_height();
	
	// Update score.
	if(matched_blocks > 0){
		grid.score += matched_blocks*grid.combo;
		if(grid.combo < MAX_COMBO) ++grid.combo;
		grid.combo_ticks = COMBO_TIMEOUT;
	} else if(grid.combo_ticks == 0){
		grid.combo = 1;
	} else {
		--grid.combo_ticks;
	}
	
	// Copy score to the screen.
	px_buffer_inc(PX_INC1);
	px_buffer_data(8, NT_ADDR(0, 10, 4));
	memset(PX.buffer, 0, 8);
	
	ultoa(grid.score, PX.buffer, 10);
	PX.buffer[4] = 'x';
	PX.buffer[5] = _hextab[grid.combo];
	PX.buffer[6] = '@';
	PX.buffer[7] = _hextab[grid.combo_ticks];
}

static void grid_tick(void){
	grid_blocks_tick();
	
	// Drop in new blocks if the field has settled.
	if(!grid_any_falling()){
		grid_drop_block();
		grid_update_fall_speed();
	}
}

uintptr_t grid_update_coro(uintptr_t _){
	while(true){
		// Look for matches while waiting for the next tick.
		for(grid.state_timer = 0; grid.state_timer < grid.block_fall_ticks; ++grid.state_timer){
			if(grid_open_chests()){
				// Prevent the timer from advancing as long as matches are happening.
				grid.state_timer = 0;
			}
			
			naco_yield(true);
		}
		
		grid_tick();
		// debug_hex((grid.combo << 4) | (grid.combo_ticks << 0));
		naco_yield(true);
		
		// Blit the blocks to the screen over several frames.
		for(grid.state_timer = 1; grid.state_timer < GRID_H - 1; ++grid.state_timer){
			px_buffer_inc(PX_INC1);
			
			for(ix = 1; ix < GRID_W - 1; ++ix){
				idx = grid_block_idx(ix, grid.state_timer);
				grid_set_block(idx, GRID[idx]);
			}
			
			naco_yield(true);
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
	
	grid.drop_cursor = 0;
	grid.column_cursor = 0;
	for(idx = 255; idx > 0; --idx){
		get_shuffled_block();
		get_shuffled_block();
		get_shuffled_column();
	}
	
	grid.garbage_mask = 0x7E;
	
	grid.speedup_counter = DROPS_PER_SPEEDUP;
	grid.block_fall_ticks = 60;
	
	grid.combo = 1;
	grid.combo_ticks = 0;
	grid.score = 0;
	
	naco_init(grid_update_coro, grid.update_coro, sizeof(grid.update_coro));
}

static const u8 MASK_BITS[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
void grid_draw_garbage(){
	ix = grid.garbage_cursor;
	if(grid.garbage_mask & MASK_BITS[ix]){
		iy = COLUMN_HEIGHT[ix];
		block_sprite(64 + ix*16, 190 - iy*16, BLOCK_GARBAGE);
	}
	
	if(++grid.garbage_cursor == GRID_W) grid.garbage_cursor = 1;
}

void grid_update(void){
	naco_resume(grid.update_coro, 0);
}
