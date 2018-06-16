#include <stdlib.h>
#include <string.h>

#include "pixler.h"
#include "shared.h"
#include "grid.h"

typedef struct {} GameState;

GameState Freeze(){
	px_wait_nmi();
	return Freeze();
}

static const u8 PALETTE[] = {
	0x1D, 0x20, 0x20, 0x20,
	0x1D, 0x06, 0x16, 0x26,
	0x1D, 0x01, 0x11, 0x21,
	0x1D, 0x09, 0x19, 0x29,
	
	0x1D, 0x20, 0x20, 0x20,
	0x1D, 0x06, 0x16, 0x26,
	0x1D, 0x01, 0x11, 0x21,
	0x1D, 0x09, 0x19, 0x29,
};

static GameState loop(){
	register u8 ticks = 0;
	
	while(true){
		// PX.scroll_y = (ticks >> 3) & 0x03;
		
		++ticks;
		px_wait_nmi();
	}
	
	return loop();
}

static GameState debug_display(){
	PPU.mask = 0x1E;
	return Freeze();
}

void px_buffer_set_metatile(u16 addr);

static GameState board(){
	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 0, 0));
	px_fill(32*30, 0x01);

	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 9, 5));
	px_fill(14, '*');
	px_addr(NT_ADDR(0, 9, 26));
	px_fill(14, '*');
	
	px_inc(PX_INC32);
	px_addr(NT_ADDR(0, 9, 6));
	px_fill(20, '*');
	px_addr(NT_ADDR(0, 22, 6));
	px_fill(20, '*');
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	// grid_set_block(0, 0, 2);
	// grid_set_block(0, 0, 0);
	// grid_set_block(1, 1, 3);
	// grid_set_block(2, 2, 4);
	// grid_set_block(3, 3, 1);
	// grid_set_block(4, 4, 2);
	// grid_set_block(5, 5, 3);
	// grid_set_block(0, 6, 4);
	// grid_set_block(1, 7, 1);
	// grid_set_block(2, 8, 2);
	// grid_set_block(3, 9, 3);

	px_buffer_inc(PX_INC1);
	px_buffer_set_metatile(NT_ADDR(0, 4, 4));
	px_buffer_set_metatile(NT_ADDR(0, 8, 20));
	// for(iy = 0; iy < 8; iy += 2){
	// 	for(ix = 0; ix < 20; ix += 2){
	// 		px_buffer_set_metatile(NT_ADDR(0, ix, iy));
	// 	}
	// }
	
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
	
	return loop();
}

GameState main(void){
	px_bank_select(0);
	px_addr(CHR_ADDR(0, 0));
	px_blit_chr(256, 0x8000);
	
	px_inc(PX_INC1);
	px_addr(0x3F00);
	px_blit(32, (u8 *)PALETTE);
	
	// return debug_chr();
	return board();
}
