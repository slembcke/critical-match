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

// Time taken by tick + blitting.
#define OVERHEAD_FRAMES (1 + GRID_H - 2)

// Min/Max waiting ticks before triggering the next fall.
#define MIN_FALL_FRAMES (15 - OVERHEAD_FRAMES)
#define MAX_FALL_FRAMES (60 - OVERHEAD_FRAMES)

// How many drops happen before speeding up.
#define DROPS_PER_SPEEDUP 16
// How many ticks to speed up the timout by.
#define FALL_TICKS_DEC 5

// How many ticks pass before the combo resets and the max value.
#define COMBO_TIMEOUT 8
#define MAX_COMBO 5
#define COMBO_LABEL_TIMEOUT 180

// How many frames pass before adding garbage blocks to the board.
#define GARBAGE_METER_TICKS 40

// How many frames garbage blocks preview for.
#define GARBAGE_PREVIEW_TIMEOUT 180

typedef struct {
	// Cursor values used for shuffling.
	u8 drop_cursor;
	u8 column_cursor;
	
	// Where and what to drop next.
	u8 queued_column;
	u8 queued_drops[2];
	
	// Game timing.
	u8 speedup_counter;
	u8 block_fall_timeout;
	
	// How many ticks left before adding garbage.
	u8 garbage_meter_ticks;
	// Frame counter for adding a garbage block to the meter.
	u8 garbage_block_timeout;
	// Queued blocks of garbage to add.
	u8 garbage_blocks;
	// +/- points to adjust garbage with.
	u8 garbage_pos_points;
	u8 garbage_neg_points;
	// Values used for previewing garbage placement.
	u8 garbage_preview_timeout;
	u8 garbage_mask;
	
	u8 combo;
	u8 combo_ticks;
	u16 score;
	
	u8 combo_label_value;
	u8 combo_label_location;
	u8 combo_label_timeout;
	
	u8 flicker_column;
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

// XOR a block with a neighbor and compare using a mask.
// Only the 'matching' bit should be left.
#define BLOCK_MATCH(block, cmp, mask, expect) (((block ^ cmp) & mask) == expect)

static bool grid_match_blocks(void){
	static u8 queue[8];
	register u8 cursor = 0;
	register u8 block;
	register u8 mask, expect;
	
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = COLUMN_HEIGHT[ix]; iy > 0; --iy){
			idx = grid_block_idx(ix, iy);
			
			// For simplicity sake, never consider the current block as matching.
			block = GRID[idx] & ~BLOCK_STATUS_MATCHING;
			
			if(block & BLOCK_STATUS_UNLOCKED){
				continue; // Skip blocks that are already unlocked.
			} else if((block & BLOCK_TYPE_MASK) != BLOCK_TYPE_OTHER){
				// Color must match, and neighbor should be matching.
				mask = BLOCK_COLOR_MASK | BLOCK_STATUS_MATCHING;
				expect = BLOCK_STATUS_MATCHING;
			} else if(block == BLOCK_GARBAGE){
				// Neighbor must be actively matching and already unlocked.
				mask = BLOCK_STATUS_UNLOCKED | BLOCK_STATUS_MATCHING;
				expect = mask;
			} else {
				// Skip other types of blocks. (empty, border)
				continue;
			}
			
			if(
				BLOCK_MATCH(block, GRID_D[idx], mask, expect) ||
				(COLUMN_HEIGHT[ix] > iy && BLOCK_MATCH(block, GRID_U[idx], mask, expect)) ||
				(COLUMN_HEIGHT_L[ix] >= iy && BLOCK_MATCH(block, GRID_L[idx], mask, expect)) ||
				(COLUMN_HEIGHT_R[ix] >= iy && BLOCK_MATCH(block, GRID_R[idx], mask, expect)) ||
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
			// Flip bits to change chests into open chests.
			block ^= BLOCK_TYPE_CHEST ^ BLOCK_TYPE_OPEN;
			
			// Set them to match against other blocks too.
			mask = BLOCK_STATUS_UNLOCKED | BLOCK_STATUS_MATCHING;
			
			// Emit coins.
			coins_add_at(idx);
		} else {
			// Keys and garbage only unlock.
			mask = BLOCK_STATUS_UNLOCKED;
		}
		
		grid_set_block(idx, block | mask);
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

static void grid_shuffle_next_drop(){
	grid.queued_column = get_shuffled_column();
	grid.queued_drops[0] = get_shuffled_block();
	grid.queued_drops[1] = get_shuffled_block();
	// TODO column
	
	buffer_set_metatile(grid.queued_drops[0] & BLOCK_GFX_MASK, NT_ADDR(0, 6, 6));
	buffer_set_metatile(grid.queued_drops[1] & BLOCK_GFX_MASK, NT_ADDR(0, 6, 4));
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
	px_buffer_inc(PX_INC1);
	px_buffer_data(5, NT_ADDR(0, 11, 4));
	memset(PX.buffer, 0, 5);
	
	// Score
	ultoa(grid.score, PX.buffer, 10);
	
	// Combo
	px_buffer_data(1, NT_ADDR(0, 21, 4));
	PX.buffer[0] = _hextab[grid.combo];
}

static void grid_remove_garbage(u8 score){
	grid.garbage_neg_points += score;
	while(grid.garbage_neg_points > MAX_COMBO){
		grid.garbage_neg_points -= MAX_COMBO;
		if(grid.garbage_blocks > 1){
			--grid.garbage_blocks;
			buffer_set_metatile(BLOCK_EMPTY, NT_ADDR(0, 6, 20 - 2*grid.garbage_blocks));
		}
	}
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
		
		grid_remove_garbage(score);
		
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

static const u8 MASK_BITS[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

// TODO Can be fairly expensive.
static void grid_shuffle_garbage(u8 count){
	while(count){
		ix = get_shuffled_column();
		if((grid.garbage_mask & MASK_BITS[ix]) == 0){
			grid.garbage_mask |= MASK_BITS[ix];
			--count;
		}
	}
}

static void grid_tick(void){
	grid_blocks_tick();
	
	// Drop in new blocks if the field has settled.
	if(!grid_any_falling()){
		grid_drop_block();
		grid_update_fall_speed();
	}
	
	if(grid.garbage_blocks > 0){
		if(++grid.garbage_meter_ticks >= GARBAGE_METER_TICKS){
			grid.garbage_meter_ticks = 0;
			
			grid.garbage_preview_timeout = GARBAGE_PREVIEW_TIMEOUT;
			grid_shuffle_garbage(grid.garbage_blocks);
			
			// Clear out the meter
			grid.garbage_blocks = 0;
			for(idx = 0; idx < 6; ++idx){
				buffer_set_metatile(BLOCK_EMPTY, NT_ADDR(0, 6, 20 - 2*idx));
			}
			
			px_wait_nmi();
		}
	} else {
		grid.garbage_meter_ticks = 0;
	}
}

uintptr_t grid_update_coro(void){
	while(true){
		// Look for matches while waiting for the next tick.
		for(grid.state_timer = 0; grid.state_timer < grid.block_fall_timeout; ++grid.state_timer){
			if(
				// Check every other frame to animate matching nicely.
				(grid.state_timer & 0x1) == 0 &&
				grid_match_blocks()
			){
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
	grid_shuffle_next_drop();
	
	grid.speedup_counter = DROPS_PER_SPEEDUP;
	grid.block_fall_timeout = MAX_FALL_FRAMES;
	
	grid.garbage_meter_ticks = 0;
	grid.garbage_block_timeout = 4*MAX_FALL_FRAMES;
	grid.flicker_column = 1;
	grid.garbage_mask = 0;
	
	grid.combo = 1;
	grid.combo_ticks = 0;
	grid.score = 0;
	
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
	
	// ticks * 2.5
	iy = grid.garbage_meter_ticks;
	iy = (u8)(4*iy + iy)/2;
	px_spr(40, 172 - iy, 0x00, 0x02);
	
	// Column warnings.
	if(COLUMN_HEIGHT[grid.flicker_column] >= GRID_H - 4){
		px_spr(68 + 16*grid.flicker_column, 48, 0x00, 0x03);
	}
	
	// TODO replace temporary sprite.
	px_spr(68 + 16*grid.queued_column, 42 + (px_ticks/8 & 0x3), 0x02, 'v');
	
	{// Combo meter.
		static const u8 SPR0[] = {0x00, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C};
		
		idx = grid.combo_ticks;
		px_spr(183, 27, 0x00, SPR0[idx]);
	}
}

void grid_draw_garbage(){
	ix = grid.flicker_column;
	if(grid.garbage_mask & MASK_BITS[ix]){
		iy = COLUMN_HEIGHT[ix];
		block_sprite(64 + ix*16, 190 - iy*16, BLOCK_GARBAGE);
	}
	
}

static void grid_place_garbage(void){
	for(ix = 1; ix < GRID_W - 1; ++ix){
		if(grid.garbage_mask & MASK_BITS[ix]){
			iy = COLUMN_HEIGHT[ix] + 1;
			
			grid_set_block(grid_block_idx(ix, iy), BLOCK_GARBAGE);
		}
	}
	
	grid.garbage_mask = 0x00;
	grid_update_column_height();
	px_wait_nmi();
}

bool grid_update(void){
	if(++grid.flicker_column == GRID_W - 1) grid.flicker_column = 1;
	
	if(grid.garbage_preview_timeout > 0){
		if(--grid.garbage_preview_timeout == 0){
			grid_place_garbage();
		}
	} else if(--grid.garbage_block_timeout == 0){
		if(grid.garbage_blocks < 6){
			buffer_set_metatile(BLOCK_GARBAGE, NT_ADDR(0, 6, 20 - 2*grid.garbage_blocks));
			++grid.garbage_blocks;
		}
		grid.garbage_block_timeout = 8*grid.block_fall_timeout;
	}
	
	return naco_resume(grid.update_coro, 0);
}
