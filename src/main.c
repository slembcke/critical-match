#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"
#include "gfx/gfx.h"

u8 joy0, joy1;

#define BG_COLOR 0x1D

const u8 GAME_PALETTE[] = {
	BG_COLOR, 0x1D, 0x28, 0x11, // blue
	BG_COLOR, 0x1D, 0x28, 0x16, // red
	BG_COLOR, 0x1D, 0x28, 0x1A, // green
	BG_COLOR, 0x1D, 0x28, 0x13, // purple

	BG_COLOR, 0x1D, 0x28, 0x11, // blue
	BG_COLOR, 0x1D, 0x28, 0x16, // red
	BG_COLOR, 0x1D, 0x28, 0x1A, // green
	BG_COLOR, 0x1D, 0x28, 0x13, // purple
};

static void wait_noinput(void){
	while(joy_read(0) || joy_read(1)){}
}

static GameState loop(void){
	px_ppu_enable();
	
	while(true){
		DEBUG_PROFILE_START();
		
		joy0 = joy_read(0);
		if(JOY_START(joy0)) return pause();
		
		grid_update();
		player_update(joy0);
		
		player_draw();
		grid_draw_garbage();
		coins_draw();
		
		px_spr_end();
		DEBUG_PROFILE_END();
		px_wait_nmi();
	}

	return loop();
}

GameState board(void){
	player_init();
	grid_init();
	coins_init();
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		// Set the palette.
		px_addr(PAL_ADDR);
		px_blit(32, (u8 *)GAME_PALETTE);

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
		
		px_wait_nmi();
	} px_ppu_enable();
	
	// GRID[grid_block_idx(1, 1)] = BLOCK_GARBAGE;
	// GRID[grid_block_idx(3, 1)] = BLOCK_KEY | BLOCK_COLOR_BLUE;
	// GRID[grid_block_idx(4, 1)] = BLOCK_KEY | BLOCK_COLOR_RED;
	// GRID[grid_block_idx(5, 1)] = BLOCK_KEY | BLOCK_COLOR_GREEN;
	// GRID[grid_block_idx(6, 1)] = BLOCK_KEY | BLOCK_COLOR_PURPLE;
	
	// GRID[grid_block_idx(3, 4)] = BLOCK_CHEST | BLOCK_COLOR_BLUE;
	// GRID[grid_block_idx(4, 4)] = BLOCK_CHEST | BLOCK_COLOR_RED;
	// GRID[grid_block_idx(5, 4)] = BLOCK_CHEST | BLOCK_COLOR_GREEN;
	// GRID[grid_block_idx(6, 4)] = BLOCK_CHEST | BLOCK_COLOR_PURPLE;

	return loop();
}

GameState pause(void){
	wait_noinput();
	
	while(!JOY_START(joy_read(0))){}
	
	wait_noinput();
	return loop();
}

GameState main_menu(void){
	static const char *msg = "MAIN  MENU";
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x80), gfx_sheet1_lz4chr, 32*16);
		decompress_lz4_to_vram(CHR_ADDR(0, 0xA0), gfx_squidman_lz4chr, 84*16);

		px_addr(PAL_ADDR);
		px_blit(32, GAME_PALETTE);
		
		px_addr(NT_ADDR(0, 0, 0));
		px_fill(32*30, 0x00);
		
		px_addr(NT_ADDR(0, 10, 12));
		px_blit(strlen(msg), msg);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	
	// Randomize the seed based on start time.
	while(true){
		for(idx = 0; idx < 255; ++idx){
			++rand_seed;
			if(JOY_START(joy_read(0))) return board();
		}
		
		px_wait_nmi();
	}
	
	return board();
}

GameState game_over(void){
	static const char *msg = "GAME OVER";
	px_ppu_disable(); {
		px_addr(NT_ADDR(0, 0, 0));
		px_fill(32*30, 0x00);
		
		px_addr(NT_ADDR(0, 10, 12));
		px_blit(strlen(msg), msg);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	
	// Wait until start is pressed.
	while(!JOY_START(joy_read(0))){}
	
	debug_freeze();
}

GameState pixelakes_screen(void){
	static const u8 PALETTE[] = {
		0x2D, 0x1D, 0x20, 0x06,
		0x2D, 0x1D, 0x10, 0x06,
		0x2D, 0x1D, 0x00, 0x06,
	};
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		px_addr(PAL_ADDR);
		px_blit(4, PALETTE + 0);
		
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_pixelakes_lz4chr, 128*16);
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_pixelakes_lz4, 1024);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	while(!JOY_START(joy_read(0))){}
	
	main_menu();
}

GameState main(void){
	joy_install(joy_static_stddrv);

	px_bank_select(0);
	
	return pixelakes_screen();
}
