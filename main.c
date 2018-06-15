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

#define NT_ADDR(tbl, x, y) (0x2000 + tbl*0x0400 + (y << 5) + x)

static u8 GRID[8*12] = {};
static u8 ATTRIBUTE_TABLE[64] = {};

#define PX_STA_BUFFER asm("sta (%v + %b), y", PX, offsetof(PX_t, buffer))

static const u8 BLOCK[][4] = {
	{0x94, 0x92, 0x91, 0x88},
	{0x94, 0x92, 0x91, 0x88},
	{0x14, 0x12, 0x11, 0x08},
	{0x14, 0x12, 0x11, 0x08},
};

static void set_block(u8 x, u8 y, u8 block){
	register const u8 *ptr = BLOCK[block];
	
	{
		static const u16 BOARD_ORIGIN = NT_ADDR(0, 10, 24);
		const u16 addr = BOARD_ORIGIN - (y << 6) + (u8)(x << 1);
		
		px_buffer_inc(PX_INC1);
		px_buffer_data(2, addr);
		asm("ldy #0");\
		asm("lda (%v), y", ptr);\
		PX_STA_BUFFER;\
		asm("ldy #1");\
		asm("lda (%v), y", ptr);\
		PX_STA_BUFFER;
		px_buffer_data(2, addr + 32);
		asm("ldy #2");\
		asm("lda (%v), y", ptr);\
		asm("ldy #0");\
		PX_STA_BUFFER;\
		asm("ldy #3");\
		asm("lda (%v), y", ptr);\
		asm("ldy #1");\
		PX_STA_BUFFER;
		
		// GRID[(u8)(8*y + x + 9)] = block;
		asm("ldy #%o", y);\
		asm("lda (sp), y");\
		asm("asl");\
		asm("asl");\
		asm("asl");\
		asm("ldy #%o", x);\
		asm("clc");\
		asm("adc (sp), y");\
		asm("adc #9");\
		asm("sta tmp1");\
		asm("ldy #%o", block);\
		asm("lda (sp), y");\
		asm("ldy tmp1");\
		asm("sta %v, y", GRID);
	}{
		static const u8 SUB_MASK[] = {0x0C, 0x03, 0xC0, 0x30};
		static const u8 PAL[] = {0x00, 0x55, 0xAA, 0xFF};
		
		u8 mask = SUB_MASK[(x & 1) | ((y & 1) << 1)]; // TODO bad ASM
		u8 offset = (u8)(((12 - y) & 0xFE) << 2) | ((u8)(x + 5) >> 1);
		u8 value = (ATTRIBUTE_TABLE[offset] & ~mask) | (PAL[block] & mask);
		ATTRIBUTE_TABLE[offset] = value;
		
		px_buffer_data(1, 0x23C0 | offset);
		asm("ldy #%o", value);\
		asm("lda (sp), y");\
		asm("ldy #0");\
		PX_STA_BUFFER;
	}
}

static GameState board(){
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
	
	set_block(0, 0, 1);
	set_block(1, 1, 2);
	set_block(2, 2, 3);
	set_block(3, 3, 0);
	set_block(4, 4, 1);
	set_block(5, 5, 2);
	set_block(0, 6, 3);
	set_block(1, 7, 0);
	set_block(2, 8, 1);
	set_block(3, 9, 2);
	
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
	
	// return debug_chr();
	return board();
}
