#include <stdlib.h>
#include <string.h>

#include "pixler.h"

typedef struct {} GameState;

static const u8 PALETTE[] = {
	0x21, 0x06, 0x06, 0x06,
};

GameState loop(){
	register u8 ticks = 0;
	
	while(true){
		PX.scroll_y = (ticks >> 3) & 0x03;
		
		++ticks;
		px_wait_nmi();
	}
	
	return loop();
}

static const char GREET[] = "Hello World! TreasureStack!";

#define NT0_ADDR(x, y) ((y << 5) + x + 0x2000)

GameState main(void){
	px_load_chr(PX_CHR_LEFT, 0, 0x80);
	
	// Enable rendering.
	PPU.mask = 0x1E;
	
	// Set the palette.
	px_buffer_data(4, 0x3F00);
	memcpy(PX.buffer, PALETTE, 4);
	
	px_set_color(0, 0x18);
	
	// Set the greeting.
	px_buffer_inc(PX_INC1);
	px_buffer_data(sizeof(GREET), NT0_ADDR(4, 15));
	memcpy(PX.buffer, GREET, sizeof(GREET));
	
	PX.scroll_x = 0;
	PX.scroll_y = 0;
	
	return loop();
}
