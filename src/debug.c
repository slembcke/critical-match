#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"

GameState debug_crash(){
	// TODO need to implement a reset vector.
	return debug_crash();
}

GameState debug_freeze(){
	px_ppu_enable();
	px_wait_nmi();
	
	
	while(true){}
	
	return debug_freeze();
}

void debug_hex(u16 value){
	px_buffer_inc(PX_INC1);
	px_buffer_data(4, NT_ADDR(0, 2, 2));
	PX.buffer[3] = _hextab[(value >> 0x0) & 0xF];
	PX.buffer[2] = _hextab[(value >> 0x4) & 0xF];
	PX.buffer[1] = _hextab[(value >> 0x8) & 0xF];
	PX.buffer[0] = _hextab[(value >> 0xC) & 0xF];
}

GameState debug_display(void){
	px_ppu_enable();
	return debug_freeze();
}

GameState debug_palette(){
	register u8 pal = 0;
	
	// Enable rendering.
	px_ppu_enable();
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
		
		// Wait until button up.
		while(joy_read(0)) px_wait_nmi();
		px_wait_nmi();
	}
	
	px_ppu_disable();
	return board();
}

static GameState debug_sprites(){
	grid_init();
	player_init();
	
	// Enable rendering.
	px_ppu_enable();
	px_wait_nmi();
	
	while(true){
		joy0 = joy_read(0);
		if(JOY_START(joy0)) break;
		
		player_tick(joy0);
		// cursor_sprite(64, 128);
		
		px_spr_end();
		px_wait_nmi();
	}
	
	px_ppu_disable();
	return board();
}

GameState debug_chr(void){
	// Top
	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 8, 6));
	px_blit(16, _hextab);
	
	// Side
	px_inc(PX_INC32);
	px_addr(NT_ADDR(0, 6, 8));
	px_blit(16, _hextab);
	
	// Grid
	px_inc(PX_INC1);
	for(iy = 0; iy < 16; ++iy){
		px_addr(NT_ADDR(0, 8, 8 + iy));
		for(ix = 0; ix < 16; ++ix){
			PPU.vram.data = ix | 16*iy;
		}
	}
	
	return debug_sprites();
}
