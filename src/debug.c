#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler.h"
#include "shared.h"

GameState Freeze(){
	px_wait_nmi();
	return Freeze();
}

GameState debug_display(void){
	PPU.mask = 0x1E;
	return Freeze();
}

void debug_sprite(u8 x, u8 y, u8 frame);

GameState debug_chr(void){
	static const char HEX[] = "0123456789ABCDEF";
	register u8 pal = 0;
	register u8 ticks = 0;
	
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
	
	idx = 0;
	while(true){
		joy0 = joy_read(0);
		
		if(JOY_START(joy0)) break;
		
		if(JOY_LEFT(joy0)) pal = (pal - 1) & 3;
		if(JOY_RIGHT(joy0)) pal = (pal + 1) & 3;
		
		px_inc(PX_INC1);
		px_buffer_data(4, PAL_ADDR);
		memcpy(PX.buffer, PALETTE + 4*pal, 4);
		
		debug_sprite(128, 32, idx);
		if((ticks & 7) == 0) ++idx;
		if(idx > 11) idx = 8;
		
		// Wait until button up.
		while(joy_read(0)) px_wait_nmi();
		
		++ticks;
		px_wait_nmi();
	}
	
	PPU.mask = 0x0;
	return board();
}
