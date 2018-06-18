#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler.h"
#include "shared.h"
#include "grid.h"

#define PAL_ADDR 0x3F00

static u8 joy0, joy1;

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

static GameState loop(void){
	while(true){
		grid_update();
		
		px_wait_nmi();
	}
	
	return loop();
}

static GameState debug_display(void){
	PPU.mask = 0x1E;
	return Freeze();
}

static GameState board(void){
	// Set the palette.
	px_inc(PX_INC1);
	px_addr(PAL_ADDR);
	px_blit(32, (u8 *)PALETTE);
	
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
	grid_set_block(3,  8, 9);
	grid_set_block(4,  6, 10);
	grid_set_block(5,  7, 11);
	grid_set_block(6,  8, 12);
	
	return loop();
}

static GameState debug_chr(void){
	static const char HEX[] = "0123456789ABCDEF";
	u8 pal = 0;
	
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
	
	while(true){
		joy0 = joy_read(0);
		
		if(JOY_START(joy0)) break;
		
		if(JOY_LEFT(joy0)) pal = (pal - 1) & 3;
		if(JOY_RIGHT(joy0)) pal = (pal + 1) & 3;
		
		px_inc(PX_INC1);
		px_buffer_data(4, PAL_ADDR);
		memcpy(PX.buffer, PALETTE + 4*pal, 4);
		// memset(PX.buffer, 0x18, 4);
		
		// Wait until button up.
		while(joy_read(0)) px_wait_nmi();
		
		px_wait_nmi();
	}
	
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
	px_addr(PAL_ADDR);
	px_blit(32, (u8 *)PALETTE);
	
	return debug_chr();
	return board();
}
