#include <stdlib.h>
#include <string.h>
#include <lz4.h>

#include "pixler.h"
#include "shared.h"

u8 GRID[GRID_W*GRID_H];
static u8 GRID_MAX_Y[GRID_W];

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

#define QUEUE_SIZE 16
#define QUEUE_INC(x) ((x + 1) & (QUEUE_SIZE - 1))
static u8 QUEUE[QUEUE_SIZE];
static u8 QUEUE_WRITE;
static u8 QUEUE_READ;

static void grid_tick(void){
	register u8 block, color, type;
	
	px_profile_enable();
	// Calculate stack heights.
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = 1; iy < GRID_H - 1; ++iy){
			idx = grid_block_idx(ix, iy);
			if(GRID[idx] == 0) break;
		}
		
		--iy;
		GRID_MAX_Y[ix] = iy;
	}
	
	for(ix = 1; ix < GRID_W - 1; ++ix){
		for(iy = GRID_MAX_Y[ix]; iy > 0; --iy){
			idx = grid_block_idx(ix, iy);
			block = GRID[idx];
			color = block & BLOCK_MASK_COLOR;
			
			if((block & BLOCK_MASK_TYPE) != BLOCK_CHEST) continue;
			
			if(
				GRID_MAX_Y[ix + 1] >= iy && (GRID[idx + 1] == (BLOCK_KEY | color) || GRID[idx + 1] == (BLOCK_OPEN | color))){
				grid_set_block(idx, BLOCK_OPEN | color);
			}
			if(
				(GRID[idx - GRID_W] == (BLOCK_KEY | color) || GRID[idx - GRID_W] == (BLOCK_OPEN | color))){
				grid_set_block(idx, BLOCK_OPEN | color);
			}
		}
	}
	
	px_profile_disable();
}

extern const u8 GRID_BIN_LZ4[];

// TODO Is this code bigger than a table? LOL
void grid_init(void){
	static const u8 row[] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
	
	decompress_lz4(GRID_BIN_LZ4, GRID, GRID_W*GRID_H);
}

void grid_update(void){
	static u8 frames = 1;
	register u8 above;
	
	if(true){
		// Debug draw stack heights.
		idx = px_ticks & 0x7;
		ix = 68 + 16*idx;
		iy = 200 - 16*GRID_MAX_Y[idx];
		px_spr(ix, iy, 0x00, '*');
	}
	
	if(frames < GRID_H - 1){
		px_buffer_inc(PX_INC1);
		
		for(ix = 1; ix < GRID_W - 1; ++ix){
			idx = grid_block_idx(ix, frames);
			
			if(GRID[idx] == 0){
				above = GRID[idx + GRID_W];
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
