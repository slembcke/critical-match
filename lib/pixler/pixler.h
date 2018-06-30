#ifndef _PIXLER_H
#define _PIXLER_H

#include <stdbool.h>
#include <stdint.h>

#include <nes.h>

typedef int8_t s8;
typedef int16_t s16;

typedef uint8_t u8;
typedef uint16_t u16;

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(x, min, max) MAX(min, MIN(x, max))

extern u8 px_mask;
#pragma zpsym("px_mask");
extern u8 px_ctrl;
#pragma zpsym("px_ctrl");

#define PX_MASK_GRAY 0x01
#define PX_MASK_BG_ENABLE 0x0E
#define PX_MASK_SPRITE_ENABLE 0x10
#define PX_MASK_RENDER_ENABLE (PX_MASK_SPRITE_ENABLE | PX_MASK_BG_ENABLE)

#define px_profile_enable() {px_mask |= PX_MASK_GRAY; PPU.mask = px_mask;}
#define px_profile_disable() {px_mask &= ~PX_MASK_GRAY; PPU.mask = px_mask;}

#define px_ppu_enable() {px_mask |= PX_MASK_RENDER_ENABLE; PPU.mask = px_mask;}
#define px_ppu_disable() {px_mask &= ~PX_MASK_RENDER_ENABLE; PPU.mask = px_mask;}

extern u8 OAM[];

extern u8 px_ticks;
#pragma zpsym("px_ticks");

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
#define PAL_ADDR 0x3F00

#define PX_INC1 0
#define PX_INC32 1

#define px_addr(addr) {	PPU.vram.address = addr >> 8; PPU.vram.address = addr & 0xFF;}
void px_inc(u8 direction);
void px_fill(u16 len, char chr);
void px_blit(u16 len, const u8 *src);
void vram_unlz4(u16 addr, void *src, u16 len);

void px_buffer_inc(u8 direction);
void px_buffer_data(u8 len, u16 addr);
void px_buffer_set_color(u8 idx, u8 color);

void px_spr(u8 x, u8 y, u8 attr, u8 chr);
void px_spr_end(void);

void px_wait_nmi(void);

#endif
