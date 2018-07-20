#ifndef _PIXLER_H
#define _PIXLER_H

#include <stdbool.h>
#include <stdint.h>

#include <nes.h>

#ifndef __CC65__
	#define asm(...)
	#define __AX__ 0
	#define __EAX__ 0
#endif

typedef int8_t s8;
typedef int16_t s16;

typedef uint8_t u8;
typedef uint16_t u16;

extern u8 OAM[256];

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

#define PX_CTRL_BASE_NAMETABLE_ADDR 0x03
#define PX_CTRL_VRAM_INC 0x04
#define PX_CTRL_SPR_TABLE_ADDR 0X08
#define PX_CTRL_BG_TABLE_ADDR 0x10

#define px_profile_start() {px_mask |= PX_MASK_GRAY; PPU.mask = px_mask;}
#define px_profile_end() {px_mask &= ~PX_MASK_GRAY; PPU.mask = px_mask;}

#define px_ppu_enable() {px_mask |= PX_MASK_RENDER_ENABLE; PPU.mask = px_mask;}
#define px_ppu_disable() {px_mask &= ~PX_MASK_RENDER_ENABLE; PPU.mask = px_mask;}

#define px_spr_table(tbl) {px_ctrl |= (tbl ? 0xFF : 0x00) & PX_CTRL_SPR_TABLE_ADDR; PPU.control = px_ctrl;}
#define px_bg_table(tbl) {px_ctrl |= (tbl ? 0xFF : 0x00) & PX_CTRL_BG_TABLE_ADDR; PPU.control = px_ctrl;}

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

void px_buffer_clear(void);
void px_buffer_inc(u8 direction);
void px_buffer_data(u8 len, u16 addr);
void px_buffer_set_color(u8 idx, u8 color);
void px_buffer_exec(void);

void px_spr_clear(void);
void px_spr(u8 x, u8 y, u8 attr, u8 chr);
void px_spr_end(void);

void px_wait_nmi(void);
void px_wait_frames(u8 frames);

void decompress_lz4_to_ram(void *dst, void *src, u16 len);
void decompress_lz4_to_vram(u16 addr, void *src, u16 len);

extern u16 rand_seed;
#pragma zpsym("rand_seed");
u8 rand8();

#endif
