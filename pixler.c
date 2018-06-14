#include "pixler.h"

void px_load_chr(u8 chr_table, u8 bank, u8 page){
	PPU.vram.address = chr_table;
	PPU.vram.address = 0x00;
	
	px_bank_select(bank);
	px_blit_pages(page, 16);
}
