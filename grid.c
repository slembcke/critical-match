#include <stdlib.h>
#include <stddef.h>

#include "pixler.h"

static u8 GRID[8*12] = {};
static u8 ATTRIBUTE_TABLE[64] = {};

#define STA_BUFFER asm("sta (%v + %b), y", PX, offsetof(PX_t, buffer))

static const u8 BLOCK[][4] = {
	{0x94, 0x92, 0x91, 0x88},
	{0x94, 0x92, 0x91, 0x88},
	{0x14, 0x12, 0x11, 0x08},
	{0x14, 0x12, 0x11, 0x08},
};

void grid_set_block(u8 x, u8 y, u8 block){
	register const u8 *ptr = BLOCK[block];
	
	{
		static const u8 BOARD_ORIGIN_X = 10;
		static const u16 BOARD_ORIGIN = NT_ADDR(0, 10, 24);
		const u16 addr = BOARD_ORIGIN - (y << 6) + (u8)(x << 1);
		
		px_buffer_inc(PX_INC1);
		px_buffer_data(2, addr);
		asm("ldy #0");\
		asm("lda (%v), y", ptr);\
		STA_BUFFER;\
		asm("ldy #1");\
		asm("lda (%v), y", ptr);\
		STA_BUFFER;
		px_buffer_data(2, addr + 32);
		asm("ldy #2");\
		asm("lda (%v), y", ptr);\
		asm("ldy #0");\
		STA_BUFFER;\
		asm("ldy #3");\
		asm("lda (%v), y", ptr);\
		asm("ldy #1");\
		STA_BUFFER;
		
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
		
		px_buffer_data(1, AT_ADDR(0) | offset);
		asm("ldy #%o", value);\
		asm("lda (sp), y");\
		asm("ldy #0");\
		STA_BUFFER;
	}
}
