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

#define COIN_COUNT 16
u8 COIN_BLOCK_IDX[COIN_COUNT];
u8 COIN_TIMEOUT[COIN_COUNT];

#define FOO(t) (-t*7/2 + t*t*2/5)
static const s8 COIN_ANIM_Y[] = {
	FOO(0x0), FOO(0x1), FOO(0x2), FOO(0x3),
	FOO(0x4), FOO(0x5), FOO(0x6), FOO(0x7),
	FOO(0x8), FOO(0x9), FOO(0xA), FOO(0xB),
	FOO(0xC), FOO(0xD), FOO(0xE), FOO(0xF),
};

static u8 coin_cursor = 0;
void coin_at(u8 idx){
	COIN_BLOCK_IDX[coin_cursor] = idx;
	COIN_TIMEOUT[coin_cursor] = 0;
	
	coin_cursor = (coin_cursor + 1) & (COIN_COUNT - 1);
}

static void draw_coins(void){
	register u8 i, t;
	
	for(i = 0; i < COIN_COUNT; ++i){
		t = COIN_TIMEOUT[i];
		
		if(t < COIN_COUNT){
			ix = (t ^ 0xFF);
			iy = COIN_ANIM_Y[t];
			
			idx = COIN_BLOCK_IDX[i];
			ix += ( 68 + (u8)((idx & 0x07) << 4));
			iy += (210 - (u8)((idx & 0xF8) << 1));
			px_spr(ix, iy, 0x00, 0x95);
			
			++COIN_TIMEOUT[i];
		}
	}
}

static GameState loop(void){
	px_ppu_enable();
	
	while(true){
		DEBUG_PROFILE_START();
		
		joy0 = joy_read(0);
		if(JOY_START(joy0)) return pause();
		
		draw_coins();
		
		grid_update();
		player_tick(joy0);
		
		px_spr_end();
		DEBUG_PROFILE_END();
		px_wait_nmi();
	}

	return loop();
}

GameState board(void){
	px_ppu_disable();
	
	player_init();
	grid_init();

	// Set the palette.
	px_inc(PX_INC1);
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

	// Enable rendering.
	px_ppu_enable();
	
	// {
	// 	u8 color = 0;
	// 	u8 counter = 0;
		
	// 	for(iy = 4; iy < GRID_H - 4; ++iy){
	// 		for(ix = 1; ix <= 5; ++ix){
	// 			// grid_set_block(grid_block_idx(ix, iy), (counter == 0 ? BLOCK_KEY : BLOCK_CHEST) | color);
	// 			GRID[grid_block_idx(ix, iy)] = (counter == 0 ? BLOCK_KEY : BLOCK_CHEST) | color;
				
	// 			color = (color + 1) & BLOCK_COLOR_MASK;
	// 			if(++counter == 7) counter = 0;
	// 		}
	// 		px_wait_nmi();
	// 	}
	// }

	return loop();
}

static GameState pause(void){
	wait_noinput();
	
	while(!JOY_START(joy_read(0))){}
	
	wait_noinput();
	return loop();
}

GameState main_menu(void){
	static const char *msg = "MAIN MENU";
	px_ppu_disable();
	
	px_addr(NT_ADDR(0, 0, 0));
	px_fill(32*30, 0x00);
	
	px_addr(NT_ADDR(0, 10, 12));
	px_blit(strlen(msg), msg);
	
	px_ppu_enable();
	px_wait_nmi();
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
	px_ppu_disable();
	
	px_addr(NT_ADDR(0, 0, 0));
	px_fill(32*30, 0x00);
	
	px_addr(NT_ADDR(0, 10, 12));
	px_blit(strlen(msg), msg);
	
	wait_noinput();
	px_wait_nmi();
	
	// Wait until start is pressed.
	while(!JOY_START(joy_read(0))){}
	
	debug_freeze();
}

GameState main(void){
	joy_install(joy_static_stddrv);

	px_bank_select(0);
	
	px_inc(PX_INC1);
	
	decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
	decompress_lz4_to_vram(CHR_ADDR(0, 0x80), gfx_sheet1_lz4chr, 32*16);
	decompress_lz4_to_vram(CHR_ADDR(0, 0xA0), gfx_squidman_lz4chr, 84*16);

	px_addr(PAL_ADDR);
	px_blit(32, (u8 *)GAME_PALETTE);
	
	// music_init(MUSIC);
	// music_play(0);

	// return debug_chr();
	return main_menu();
}
