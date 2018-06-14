#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

#include <nes.h>

extern struct {
	u16 scroll_x;
	u16 scroll_y;
	u8 *buffer;
} PX;
#pragma zpsym("PX");

void px_bank_select(u8 bank);
void _px_blit_pages(u16 count8_basepage8);
#define px_blit_pages(basepage, count) _px_blit_pages(((count) << 8) | (basepage & 0xFF))

#define PX_CHR_LEFT 0x00
#define PX_CHR_RIGHT 0x10
void px_load_chr(u8 chr_table, u8 rom_bank, u8 page);

#define PX_INC1 0
#define PX_INC32 1

void px_inc(u8 direction);
void px_fill(char chr, u16 len);

void px_buffer_inc(u8 direction);

void px_buffer_data(u8 len, u16 addr);
void px_set_color(u8 idx, u8 color);

void px_wait_nmi();
