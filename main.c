#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler.h"
#include "shared.h"
#include "grid.h"

typedef struct {} GameState;

GameState Freeze(){
	px_wait_nmi();
	return Freeze();
}

static const u8 PALETTE[] = {
	0x1D, 0x1D, 0x28, 0x12, // blue
	0x1D, 0x1D, 0x28, 0x06, // red
	0x1D, 0x1D, 0x28, 0x19, // green
	0x1D, 0x1D, 0x28, 0x13, // purple
	
	0x1D, 0x20, 0x20, 0x20,
	0x1D, 0x06, 0x16, 0x26,
	0x1D, 0x01, 0x11, 0x21,
	0x1D, 0x09, 0x19, 0x29,
};

static void grid_set_block(u8 x, u8 y, u8 idx){
	px_buffer_inc(PX_INC1);
	px_buffer_set_metatile(idx, NT_ADDR(0, 8 + 2*x, 26 - 2*y));
	
	GRID[y][x] = idx;
}

static void grid_tick_fall(){
	static ticks = 1;
	
	if(ticks < GRID_H - 1){
		px_buffer_inc(PX_INC1);
		
		for(ix = 1; ix < GRID_W - 1; ++ix){
			u8 above = GRID[ticks + 1][ix];
			if(GRID[ticks][ix] == 0 && above != 0){
				grid_set_block(ix, ticks, above);
				grid_set_block(ix, ticks + 1, 0);
			}
		}
	}
	
	++ticks;
	if(ticks >= 32) ticks = 1;
}

static GameState loop(){
	while(true){
		grid_tick_fall();
		px_wait_nmi();
	}
	
	return loop();
}

static GameState debug_display(){
	PPU.mask = 0x1E;
	return Freeze();
}

static GameState board(){
	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 0, 0));
	px_fill(32*30, 0x00);
	
	// Top edge.
	px_addr(NT_ADDR(0, 10, 5));
	px_fill(12, 0x0B);
	
	// Bottom edge.
	px_addr(NT_ADDR(0, 10, 26));
	px_fill(12, 0x0B);
	
	px_inc(PX_INC32);
	
	// Left edge.
	px_addr(NT_ADDR(0, 9, 6));
	px_fill(20, 0x0E);
	
	// Right edge.
	px_addr(NT_ADDR(0, 22, 6));
	px_fill(20, 0x0E);
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	grid_set_block(1, 10, 1);
	grid_set_block(1,  9, 2);
	grid_set_block(1,  8, 3);
	grid_set_block(2, 10, 4);
	grid_set_block(2,  8, 5);
	grid_set_block(3,  8, 5);
	grid_set_block(4,  6, 6);
	grid_set_block(5,  7, 7);
	grid_set_block(6,  8, 8);
	
	return loop();
}

static GameState debug_chr(){
	static const char HEX[] = "0123456789ABCDEF";
	
	// Top
	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 8, 6));
	px_blit(16, HEX);
	
	// Side
	px_inc(PX_INC32);
	px_addr(NT_ADDR(0, 6, 8));
	px_blit(16, HEX);
	
	// Grid
	px_inc(PX_INC1);
	for(iy = 0; iy < 16; ++iy){
		px_addr(NT_ADDR(0, 8, 8 + iy));
		for(ix = 0; ix < 16; ++ix){
			PPU.vram.data = ix | 16*iy;
		}
	}
	
	// Enable rendering.
	PPU.mask = 0x1E;
	px_wait_nmi();
	
	while(!JOY_START(joy_read(0))){}
	while(JOY_START(joy_read(0))){}
	
	PPU.mask = 0x0;
	return board();
}

extern u8 neschar_inc[];
extern u8 gfx_sheet1_chr[];

GameState main(void){
	joy_install(joy_static_stddrv);
	
	px_bank_select(0);
	px_addr(CHR_ADDR(0, 0));
	px_blit_chr(256, neschar_inc);
	px_addr(CHR_ADDR(0, 0x80));
	px_blit_chr(128, gfx_sheet1_chr);
	
	px_inc(PX_INC1);
	px_addr(0x3F00);
	px_blit(32, (u8 *)PALETTE);
	
	return debug_chr();
	return board();
}
