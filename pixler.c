#include "pixler.h"

void px_load_chr(u8 table, u8 bank, u8 page){
	PPU.vram.address = table;
	PPU.vram.address = 0x00;
	
	px_bank_select(bank);
	px_blit_pages(page, 16);
}
