#include <stdlib.h>
#include <string.h>

#include "pixler.h"

typedef struct {} GameState;

static u8 i, ix, iy;

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

static const u16 NT_BASE[] = {0x2000, 0x2400, 0x2800, 0x2C00};
#define NT_ADDR(tbl, x, y) (NT_BASE[tbl] + (y << 5) + x)

static void attr_pal(u8 x, u8 y, u8 pal){
	static const u8 SUB_MASK[] = {0x03, 0x0C, 0x30, 0xC0};
	static const u8 PAL[] = {0x00, 0x55, 0xAA, 0xFF};
	
	u8 mask = SUB_MASK[(x & 1) | ((y & 1) << 1)];
	u16 addr = 0x23C0 | (u8)((y & 0xFE) << 2) | (u8)(x >> 1);
	u8 value;
	
	px_addr(addr);
	// First read resets the IO register and returns garbage.
	value = PPU.vram.data;
	value = PPU.vram.data;
	
	px_addr(addr);
	PPU.vram.data = (value & ~mask) | (PAL[pal] & mask);
}

u8 attributes[64] = {};

static void set_block(u8 x, u8 y, u8 block){
	const u16 addr = NT_ADDR(0, 10, 5) + (u8)(y << 6) + (u8)(x << 1);
	
	px_buffer_inc(PX_INC1);
	px_buffer_data(2, addr);
	PX.buffer[0] = 'A';
	PX.buffer[1] = 'B';
	px_buffer_data(2, addr + 32);
	PX.buffer[0] = 'C';
	PX.buffer[1] = 'D';
}

static GameState board(){
	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 9, 4));
	px_fill(14, '*');
	px_addr(NT_ADDR(0, 9, 25));
	px_fill(14, '*');
	
	px_inc(PX_INC32);
	px_addr(NT_ADDR(0, 9, 5));
	px_fill(20, '*');
	px_addr(NT_ADDR(0, 22, 5));
	px_fill(20, '*');
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	set_block(0, 0, 1);
	set_block(1, 1, 2);
	
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
	
	// Set palette
	// px_addr(NT_ADDR(0, 0, 30));
	// px_fill(64, 0x55);
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	return loop();
}

GameState main(void){
	px_load_chr(PX_CHR_LEFT, 0, 0x80);
	
	px_inc(PX_INC1);
	px_addr(0x3F00);
	px_blit(32, (u8 *)PALETTE);
	
	return board();
	// return debug_chr();
}
