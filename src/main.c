#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
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
	px_ppu_enable();
	
	while(true){
		// px_profile_enable();
		
		joy0 = joy_read(0);

		grid_update();
		player_tick(joy0);
		
		px_spr_end();
		px_profile_disable();
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
	px_ppu_enable();
	
	{
		u8 color = BLOCK_PURPLE;
		u8 block = BLOCK_CHEST | color;
		grid_set_block(grid_block_idx(1, 3), block);
		grid_set_block(grid_block_idx(1, 4), block);
		grid_set_block(grid_block_idx(1, 5), block);
		grid_set_block(grid_block_idx(1, 6), block);
		grid_set_block(grid_block_idx(1, 7), block);
		px_wait_nmi();
		
		grid_set_block(grid_block_idx(2, 3), block);
		grid_set_block(grid_block_idx(2, 4), block);
		grid_set_block(grid_block_idx(2, 5), block);
		grid_set_block(grid_block_idx(2, 6), block);
		grid_set_block(grid_block_idx(2, 7), block);
		px_wait_nmi();
		
		grid_set_block(grid_block_idx(3, 3), block);
		grid_set_block(grid_block_idx(3, 4), block);
		grid_set_block(grid_block_idx(3, 5), BLOCK_KEY | color);
		grid_set_block(grid_block_idx(3, 6), block);
		grid_set_block(grid_block_idx(3, 7), block);
		px_wait_nmi();
		
		grid_set_block(grid_block_idx(4, 3), block);
		grid_set_block(grid_block_idx(4, 4), block);
		grid_set_block(grid_block_idx(4, 5), block);
		grid_set_block(grid_block_idx(4, 6), block);
		grid_set_block(grid_block_idx(4, 7), block);
		px_wait_nmi();
		
		grid_set_block(grid_block_idx(5, 3), block);
		grid_set_block(grid_block_idx(5, 4), block);
		grid_set_block(grid_block_idx(5, 5), block);
		grid_set_block(grid_block_idx(5, 6), block);
		grid_set_block(grid_block_idx(5, 7), block);
		px_wait_nmi();
		
	}

	return loop();
}

extern u8 gfx_neschar_lz4chr[];
extern u8 gfx_sheet1_lz4chr[];
extern u8 gfx_squidman_lz4chr[];

GameState main(void){
	joy_install(joy_static_stddrv);

	px_bank_select(0);
	
	px_inc(PX_INC1);
	
	vram_unlz4(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
	vram_unlz4(CHR_ADDR(0, 0x80), gfx_sheet1_lz4chr, 32*16);
	vram_unlz4(CHR_ADDR(0, 0xA0), gfx_squidman_lz4chr, 84*16);

	px_addr(PAL_ADDR);
	px_blit(32, (u8 *)PALETTE);
	
	// music_init(MUSIC);
	// music_play(0);

	// return debug_chr();
	return board();
}
