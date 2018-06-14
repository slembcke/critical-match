#include <stdlib.h>
#include <string.h>

#include "pixler.h"

typedef struct {} GameState;

static u8 i, ix, iy;

static const u8 PALETTE[] = {
	0x1D, 0x20, 0x20, 0x20,
	0x1D, 0x20, 0x20, 0x20,
	0x1D, 0x20, 0x20, 0x20,
	0x1D, 0x20, 0x20, 0x20,
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

GameState main(void){
	px_load_chr(PX_CHR_LEFT, 0, 0x80);
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	// Load the palette.
	px_buffer_data(4, 0x3F00);
	memcpy(PX.buffer, PALETTE, 4);
	
	px_buffer_inc(PX_INC1);
	// Top bar
	px_buffer_data(14, NT0_ADDR(9, 4));
	memset(PX.buffer, '*', 14);
	px_wait_nmi();
	// Bottom bar
	px_buffer_data(14, NT0_ADDR(9, 25));
	memset(PX.buffer, '*', 14);
	px_wait_nmi();

	px_buffer_inc(PX_INC32);
	// Left bar
	px_buffer_data(20, NT0_ADDR(9, 5));
	memset(PX.buffer, '*', 20);
	px_wait_nmi();
	// Right bar
	px_buffer_data(20, NT0_ADDR(22, 5));
	memset(PX.buffer, '*', 20);
	px_wait_nmi();
	
	px_buffer_inc(PX_INC1);
	for(i = 0; i < 20; i++){
		px_buffer_data(12, NT0_ADDR(10, i + 5));
		memcpy(PX.buffer, TMP_DATA + ((i >> 1) << 1), 12);
		px_wait_nmi();
	}
	
	return loop();
}
