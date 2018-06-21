#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler.h"
#include "shared.h"

u8 joy0, joy1;

#define BG_COLOR 0x1D

const u8 PALETTE[] = {
	BG_COLOR, 0x1D, 0x28, 0x12, // blue
	BG_COLOR, 0x1D, 0x28, 0x06, // red
	BG_COLOR, 0x1D, 0x28, 0x19, // green
	BG_COLOR, 0x1D, 0x28, 0x13, // purple

	BG_COLOR, 0x1E, 0x28, 0x12, // blue
	BG_COLOR, 0x1E, 0x28, 0x06, // red
	BG_COLOR, 0x1E, 0x28, 0x19, // green
	BG_COLOR, 0x1E, 0x28, 0x13, // purple
};

static GameState loop(void){
	while(true){
		PPU.mask = 0x1E | 1;

		joy0 = joy_read(0);

		grid_update();
		player_tick(joy0);
		
		PPU.mask = 0x1E;
		px_wait_nmi();
	}

	return loop();
}

GameState board(void){
	player_init();
	grid_init();

	// Set the palette.
	px_inc(PX_INC1);
	px_addr(PAL_ADDR);
	px_blit(32, (u8 *)PALETTE);

	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 0, 0));
	px_fill(32*30, 0x00);

	// Top edge.
	px_addr(NT_ADDR(0, 9, 5));
	px_fill(14, 0x09);

	// Bottom edge.
	px_addr(NT_ADDR(0, 9, 26));
	px_fill(14, 0x09);

	px_inc(PX_INC32);

	// Left edge.
	px_addr(NT_ADDR(0, 9, 6));
	px_fill(20, 0x09);

	// Right edge.
	px_addr(NT_ADDR(0, 22, 6));
	px_fill(20, 0x09);

	// Enable rendering.
	PPU.mask = 0x1E;

	grid_set_block(1, 10, 1);
	grid_set_block(1,  9, 2);
	grid_set_block(1,  8, 3);
	grid_set_block(2, 10, 4);
	grid_set_block(2,  8, 5);
	grid_set_block(3,  8, 9);
	grid_set_block(4,  6, 10);
	grid_set_block(5,  7, 11);
	grid_set_block(3,  3, 12);

	return loop();
}

extern u8 neschar_inc[];
extern u8 gfx_sheet1_chr[];
extern u8 gfx_squidman_chr[];

GameState main(void){
	joy_install(joy_static_stddrv);

	px_bank_select(0);
	px_addr(CHR_ADDR(0, 0));
	px_blit_chr(256, neschar_inc);
	px_addr(CHR_ADDR(0, 0x80));
	px_blit_chr(128, gfx_sheet1_chr);
	px_addr(CHR_ADDR(0, 0xA0));
	px_blit_chr(128, gfx_squidman_chr);

	px_inc(PX_INC1);
	px_addr(PAL_ADDR);
	px_blit(32, (u8 *)PALETTE);
	
	music_init(MUSIC);
	music_play(0);

	return debug_chr();
	return board();
}
