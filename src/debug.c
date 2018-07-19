#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"

void debug_crash(){
	while(true){}
}

void debug_freeze(){
	// Finish the current frame.
	px_ppu_enable();
	px_wait_nmi();
	
	// Thene loop forever.
	while(true){}
}

void debug_hex(u16 value){
	px_buffer_inc(PX_INC1);
	px_buffer_data(4, NT_ADDR(0, 2, 2));
	PX.buffer[3] = _hextab[(value >> 0x0) & 0xF];
	PX.buffer[2] = _hextab[(value >> 0x4) & 0xF];
	PX.buffer[1] = _hextab[(value >> 0x8) & 0xF];
	PX.buffer[0] = _hextab[(value >> 0xC) & 0xF];
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
	
	debug_freeze();
}
