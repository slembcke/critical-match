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

GameState loop(){
	register u8 ticks = 0;
	
	while(true){
		// PX.scroll_y = (ticks >> 3) & 0x03;
		
		++ticks;
		px_wait_nmi();
	}
	
	return loop();
}

static const char GREET[] = "Hello World! TreasureStack!";

#define NT0_ADDR(x, y) ((y << 5) + x + 0x2000)

static char *TMP_DATA = "AABBCCDDEEFFGGHHIIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ";

GameState board(){
	px_inc(PX_INC1);
	px_addr(NT0_ADDR(9, 4));
	px_fill(14, '*');
	px_addr(NT0_ADDR(9, 25));
	px_fill(14, '*');
	
	px_inc(PX_INC32);
	px_addr(NT0_ADDR(9, 5));
	px_fill(20, '*');
	px_addr(NT0_ADDR(22, 5));
	px_fill(20, '*');
	
	px_inc(PX_INC1);
	for(i = 0; i < 20; i++){
		px_addr(NT0_ADDR(10, 5) + i*32);
		px_blit(12, TMP_DATA + ((i >> 1) << 1));
	}
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	return loop();
}

static char HEX[] = "0123456789ABCDEF";

GameState debug_chr(){
	px_inc(PX_INC1);
	for(i = 0; i < 8; ++i){
		px_addr(NT0_ADDR(0, i)); px_fill(8, 0x10);
	}
	
	// Top
	px_inc(PX_INC1);
	px_addr(NT0_ADDR(8, 6));
	px_blit(sizeof(HEX), HEX);
	
	// Side
	px_inc(PX_INC32);
	px_addr(NT0_ADDR(6, 8));
	px_blit(sizeof(HEX), HEX);
	
	// Grid
	px_inc(PX_INC1);
	for(iy = 0; iy < 16; ++iy){
		px_addr(NT0_ADDR(8, 8 + iy));
		for(ix = 0; ix < 16; ++ix){
			PPU.vram.data = ix | 16*iy;
		}
	}
	
	// Set palette
	// px_addr(NT0_ADDR(0, 30));
	// px_fill(64, 0xE4);
	{
		u8 x = 0;
		u8 y = 2;
		u8 pal = 1;
		u16 addr = 0x23C0 + (x >> 1) + ((y & 0xFE) << 2);
		px_addr(addr);
		PPU.vram.data = 0x55;
	}
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	return loop();
}

GameState main(void){
	px_load_chr(PX_CHR_LEFT, 0, 0x80);
	
	px_inc(PX_INC1);
	px_addr(0x3F00);
	px_blit(32, (u8 *)PALETTE);
	
	// return board();
	return debug_chr();
}
