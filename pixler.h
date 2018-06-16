#ifndef _PIXLER_H
#define _PIXLER_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

#include <nes.h>

typedef struct {
	u16 scroll_x;
	u16 scroll_y;
	u8 * const buffer;
} PX_t;

extern PX_t PX;
#pragma zpsym("PX");

void px_bank_select(u8 bank);

#define CHR_ADDR(bank, chr) (bank*0x1000 | chr*0x10)
#define NT_ADDR(tbl, x, y) (0x2000 | tbl*0x400 | (y << 5) | x)
#define AT_ADDR(tbl) (0x23C0 + tbl*0x400)

#define PX_INC1 0
#define PX_INC32 1

#define px_addr(addr) {	PPU.vram.address = addr >> 8; PPU.vram.address = addr & 0xFF;}
void px_inc(u8 direction);
void px_fill(u16 len, char chr);
void px_blit(u16 len, const u8 *src);
#define px_blit_chr(count, src) {px_inc(PX_INC1); px_blit(count << 4, src);}

void px_buffer_inc(u8 direction);
void px_buffer_data(u8 len, u16 addr);
void px_buffer_set_color(u8 idx, u8 color);

void px_wait_nmi();

#endif
