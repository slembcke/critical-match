#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lz4.h>

#include "pixler/pixler.h"
#include "naco/naco.h"
#include "shared.h"
#include "gfx/gfx.h"

// Block values in the grid.
u8 GRID[GRID_BYTES];

// Height of each stacked column.
u8 COLUMN_HEIGHT[GRID_W];

// Time taken by tick + blitting.
#define OVERHEAD_FRAMES (1 + GRID_H - 2)

// Min/Max waiting ticks before triggering the next fall.
// TODO should be 15, but blitting takes too long.
#define MIN_FALL_FRAMES (16 - OVERHEAD_FRAMES)
#define MAX_FALL_FRAMES (60 - OVERHEAD_FRAMES)

// How many drops happen before speeding up.
#define DROPS_PER_SPEEDUP 16
// How many ticks to speed up the timout by.
#define FALL_TICKS_DEC 5

// How many ticks pass before the combo resets and the max value.
#define COMBO_TIMEOUT 8
#define MAX_COMBO 5
#define COMBO_LABEL_TIMEOUT 180

typedef struct {
	// Cursor values used for shuffling.
	u8 drop_cursor;
	u8 column_cursor;
	
	// Where and what to drop next.
	u8 queued_column;
	u8 queued_drops[2];
	
	u8 shape;
	
	// Game timing.
	u8 speedup_counter;
	u8 block_fall_timeout;
	
	u8 combo;
	u8 combo_ticks;
	u16 score;
	
	u8 combo_label_value;
	u8 combo_label_location;
	u8 combo_label_timeout;
	
	u8 flicker_column;
	u8 state_timer;
	u8 update_coro[16];
	
	u8 pause_semaphore;
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

bool grid_match_tetromino(u8 shape);

static bool grid_match_blocks(void){
	register u8 block;
	
	for(ix = GRID_W - 2; ix > 0; --ix){
		for(iy = COLUMN_HEIGHT[ix]; iy > 0; --iy){
			idx = grid_block_idx(ix, iy);
			
			block = GRID[idx];
			
			if(
				// TODO check block type?
				(block & BLOCK_STATUS_UNLOCKED) == 0
				&& grid_match_tetromino(grid.shape)
			){
				return true;
			}
		}
	}
	
	return false;
}

void grid_update_column_height(void){
	for(ix = GRID_W - 2; ix > 0; --ix){
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

// TODO This generates terrible code. Rewrite in asm?
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
		0, 1, 2, 3,
		0, 1, 2, 3,
		0, 1, 2, 3,
		0, 1, 2, 3,
	};

	static const u8 BLOCKS[] = {
		BLOCK_CHEST | BLOCK_COLOR_BLUE,
		BLOCK_CHEST | BLOCK_COLOR_RED,
		BLOCK_CHEST | BLOCK_COLOR_GREEN,
		BLOCK_CHEST | BLOCK_COLOR_PURPLE,
	};
	
	return BLOCKS[lru_shuffle(DROPS, sizeof(DROPS), 0x7, &grid.drop_cursor)];
}

static u8 get_shuffled_column(void){
	static u8 COLUMNS[] = {1, 2, 3, 4, 5, 6};
	return lru_shuffle(COLUMNS, sizeof(COLUMNS), 0x3, &grid.column_cursor);
}

static void grid_shuffle_next_drop(){
	grid.queued_column = get_shuffled_column();
	grid.queued_drops[0] = get_shuffled_block();
	grid.queued_drops[1] = get_shuffled_block();
	// TODO column
	
	buffer_set_metatile(grid.queued_drops[0] & BLOCK_GFX_MASK, NT_ADDR(0, 6, 8));
	buffer_set_metatile(grid.queued_drops[1] & BLOCK_GFX_MASK, NT_ADDR(0, 6, 6));
}

static void grid_drop_block(void){
	idx = grid_block_idx(grid.queued_column, GRID_H - 2);
	
	// Game over if the drop location isn't clear.
	if(GRID[idx] != BLOCK_EMPTY) naco_yield(false);
	
	// Push the first block directly onto the screen.
	grid_set_block(idx, grid.queued_drops[0]);
	
	// Write the second block into GRID and let it fall onto the screen.
	idx = grid_block_idx(grid.queued_column, GRID_H - 1);
	GRID[idx] = grid.queued_drops[1];
	
	grid_shuffle_next_drop();
}

static void grid_update_fall_speed(void){
	--grid.speedup_counter;
	if(grid.speedup_counter == 0){
		grid.block_fall_timeout -= FALL_TICKS_DEC;
		if(grid.block_fall_timeout < MIN_FALL_FRAMES){
			grid.block_fall_timeout = MIN_FALL_FRAMES;
		}
		
		grid.speedup_counter = DROPS_PER_SPEEDUP;
	}
}

static void grid_blit(void){
	// Copy score to the screen.
	grid_buffer_score(NT_ADDR(0, 11, 4));
	
	// Combo
	px_buffer_data(1, NT_ADDR(0, 21, 4));
	PX.buffer[0] = _hextab[grid.combo];
}

static void grid_blit_shape(u8 shape){
	px_buffer_data(4, NT_ADDR(0, 24, 7));
	memcpy(PX.buffer, gfx_shapes + 8*shape + 8, 4);
	px_buffer_data(4, NT_ADDR(0, 24, 8));
	memcpy(PX.buffer, gfx_shapes + 8*shape + 12, 4);
	
	// px_addr(NT_ADDR(0, 24, 7));
	// px_blit(4, gfx_shapes + 8*shape + 8);
	// px_addr(NT_ADDR(0, 24, 8));
	// px_blit(4, gfx_shapes + 8*shape + 12);
}

static void grid_blocks_tick(void){
	// High bit marks any block. Low 7 count unlocked chests.
	register u8 matched_blocks = 0;
	register u8 block;
	
	for(iy = 1; iy < GRID_H - 1; ++iy){
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, iy);
			
			if(GRID[idx] == BLOCK_EMPTY && GRID_U[idx] != BLOCK_EMPTY){
				// Block is unsupported, make it fall.
				block = GRID_U[idx];
				GRID[idx] = block;
				GRID_U[idx] = BLOCK_EMPTY;
			} else if(GRID[idx] & BLOCK_STATUS_UNLOCKED){
				// Match out unlocked blocks.
				grid.combo_label_location = idx;
				matched_blocks |= 0x80;
				if(GRID[idx] & BLOCK_TYPE_CHEST) ++matched_blocks;
				
				GRID[idx] = BLOCK_EMPTY;
			}
		}
	}
	
	grid_update_column_height();
	
	// Update score.
	if(matched_blocks > 0){
		u8 score = (matched_blocks & 0x7F)*grid.combo;
		grid.score += score;
		
		grid.combo_label_value = grid.combo;
		grid.combo_label_timeout = COMBO_LABEL_TIMEOUT;
		
		if(grid.combo < MAX_COMBO) ++grid.combo;
		grid.combo_ticks = COMBO_TIMEOUT;
	} else if(grid.combo_ticks == 0){
		grid.combo = 1;
	} else {
		--grid.combo_ticks;
	}
	
	grid_blit();
}

static void grid_tick(void){
	grid_blocks_tick();
	
	// Drop in new blocks if the field has settled.
	if(!grid_any_falling()){
		grid_drop_block();
		grid_update_fall_speed();
	}
}

uintptr_t grid_update_coro(void){
	grid_blit_shape(0);
	
	while(true){
		// Look for matches while waiting for the next tick.
		for(grid.state_timer = 0; grid.state_timer < grid.block_fall_timeout; ++grid.state_timer){
			if(grid_match_blocks()){
				grid.shape = (grid.shape + 1) & 0x3;
				grid_blit_shape(grid.shape);
				
				// Prevent the timer from advancing as long as matches are happening.
				grid.state_timer = 0;
			}
			
			naco_yield(true);
		}
		
		// Don't tick during some animations and other events.
		while(grid.pause_semaphore > 0) naco_yield(true);
		
		grid_tick();
		// debug_hex((grid.combo << 4) | (grid.combo_ticks << 0));
		naco_yield(true);
		
		// Blit the blocks to the screen over several frames.
		for(grid.state_timer = GRID_H - 2; grid.state_timer > 0; --grid.state_timer){
			px_buffer_inc(PX_INC1);
			
			for(ix = GRID_W - 2; ix > 0; --ix){
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
	memset(COLUMN_HEIGHT, 0x0, sizeof(COLUMN_HEIGHT));
	memset(&grid, 0x0, sizeof(grid));
	for(idx = 255; idx > 0; --idx){
		get_shuffled_block();
		get_shuffled_block();
		get_shuffled_column();
	}
	grid_shuffle_next_drop();
	
	grid.speedup_counter = DROPS_PER_SPEEDUP;
	grid.block_fall_timeout = MAX_FALL_FRAMES;
	
	grid.flicker_column = GRID_W - 2;
	
	grid.combo = 1;
	
	naco_init((naco_func)grid_update_coro, grid.update_coro, sizeof(grid.update_coro));
}

void grid_draw_indicators(void){
	if(grid.combo_label_timeout > 0){
		ix = grid_block_x(grid.combo_label_location,  4);
		iy = grid_block_y(grid.combo_label_location, -4);
		iy -= (u8)(COMBO_LABEL_TIMEOUT - grid.combo_label_timeout)/4;
		px_spr(ix, iy, (px_ticks >> 2) & 0x3, 0x7A + grid.combo_label_value);
		
		--grid.combo_label_timeout;
	}
	
	// Column warnings.
	if(COLUMN_HEIGHT[grid.flicker_column] >= GRID_H - 4){
		px_spr(68 + 16*grid.flicker_column, 50, 0x03, 0x03);
	}
	
	// Draw drop indicator.
	px_spr(68 + 16*grid.queued_column, 46 + bounce4(), 0x01, 0x07);
	
	{// Combo meter.
		static const u8 SPR0[] = {0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
		
		idx = grid.combo_ticks;
		px_spr(183, 27, 0x02, SPR0[idx]);
	}
}

bool grid_update(void){
	if(--grid.flicker_column == 0) grid.flicker_column = GRID_W - 2;
	
	return naco_resume(grid.update_coro, 0);
}

void grid_pause_semaphore(s8 inc){
	grid.pause_semaphore += inc;
}

// Rewrite?
void grid_buffer_score(u16 addr){
	px_buffer_inc(PX_INC1);
	px_buffer_data(5, addr);
	memset(PX.buffer, 0x00, 5);
	ultoa(grid.score, PX.buffer, 10);
	for(idx = 4; PX.buffer[idx] == 0; --idx) PX.buffer[idx] = 0x20;
}
