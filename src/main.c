#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"
#include "gfx/gfx.h"

u8 joy0, joy1;

#define BG_COLOR 0x1D

static void blit_palette(void){
	const u8 PALETTE[] = {
		BG_COLOR, 0x1D, 0x28, 0x11, // blue
		BG_COLOR, 0x1D, 0x28, 0x16, // red
		BG_COLOR, 0x1D, 0x28, 0x1A, // green
		BG_COLOR, 0x1D, 0x28, 0x13, // purple

		BG_COLOR, 0x1D, 0x28, 0x11, // blue
		BG_COLOR, 0x1D, 0x28, 0x16, // red
		BG_COLOR, 0x1D, 0x28, 0x1A, // green
		BG_COLOR, 0x1D, 0x28, 0x13, // purple
	};
	
	px_addr(PAL_ADDR);
	px_blit(sizeof(PALETTE), PALETTE);
}

static void wait_noinput(void){
	while(joy_read(0) || joy_read(1)){}
}

static GameState main_menu(void);
static void pause(void);
static GameState game_over(void);

static GameState game_loop(void){
	player_init();
	grid_init();
	coins_init();
	
	// GRID[grid_block_idx(1, 1)] = BLOCK_GARBAGE;
	// GRID[grid_block_idx(3, 1)] = BLOCK_KEY | BLOCK_COLOR_BLUE;
	// GRID[grid_block_idx(4, 1)] = BLOCK_KEY | BLOCK_COLOR_RED;
	// GRID[grid_block_idx(5, 1)] = BLOCK_KEY | BLOCK_COLOR_GREEN;
	// GRID[grid_block_idx(6, 1)] = BLOCK_KEY | BLOCK_COLOR_PURPLE;
	
	// GRID[grid_block_idx(3, 4)] = BLOCK_CHEST | BLOCK_COLOR_BLUE;
	// GRID[grid_block_idx(4, 4)] = BLOCK_CHEST | BLOCK_COLOR_RED;
	// GRID[grid_block_idx(5, 4)] = BLOCK_CHEST | BLOCK_COLOR_GREEN;
	// GRID[grid_block_idx(6, 4)] = BLOCK_CHEST | BLOCK_COLOR_PURPLE;
	
	// for(idx = 8; idx < GRID_BYTES - 16; ++idx){
	// 	GRID[idx] = BLOCK_GARBAGE;
	// }
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		blit_palette();
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x80), gfx_sheet1_lz4chr, 32*16);
		
		px_spr_table(1);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x20), gfx_explosion_lz4chr, 32*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0xA0), gfx_squidman_lz4chr, 84*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x80), gfx_sheet1_lz4chr, 32*16);
		
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_board_lz4, 1024);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	while(true){
		DEBUG_PROFILE_START();
		
		joy0 = joy_read(0);
		joy1 = joy_read(1);
		if(JOY_START(joy0)) pause();
		
		if(!grid_update()) break;
		player_update(joy0);
		
		player_draw_blocks();
		player_draw();
		grid_draw_indicators();
		grid_draw_garbage();
		coins_draw();
		
		player_draw_grapple();
		
		px_spr_end();
		DEBUG_PROFILE_END();
		px_wait_nmi();
	}
	
	px_wait_nmi();
	return game_over();
}

static void pause(void){
	wait_noinput();
	while(!JOY_START(joy_read(0))){}
	wait_noinput();
}

// TODO This is pretty terrible.
static GameState game_over(void){
	u8 boom[16] = {};
	register u8 y;
	
	px_spr_clear();
	px_wait_nmi();
	
	for(ix = 0; ix < 255; ++ix){
		idx = rand8();
		if(idx - 8 < GRID_BYTES - 16 && (idx & (sizeof(boom) - 1)) - 1 < 6){
			boom[ix & (sizeof(boom) - 1)] = idx;
			grid_set_block(idx, BLOCK_EMPTY);
		} else {
			boom[ix & (sizeof(boom) - 1)] = 0;
		}
		
		for(iy = 0; iy < sizeof(boom); ++iy){
			idx = boom[iy];
			if(idx) explosion_sprite(grid_block_x(idx, 0), grid_block_y(idx, -6), ((ix + iy)/2) & 7);
		}
		
		px_spr_end();
		px_wait_frames(2);
	}
	
	px_buffer_inc(PX_INC1);
	px_ppu_disable(); {
	
		px_addr(NT_ADDR(0, 10, 12));
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_game_over_lz4, 1024);
		
		// Score
		px_buffer_data(5, NT_ADDR(0, 17, 14));
		memset(PX.buffer, 0, 5);
		ultoa(grid_get_score(), PX.buffer, 10);
		
		px_spr_clear();
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	
	// Wait until start is pressed.
	while(!JOY_START(joy_read(0))){}
	
	return main_menu();
}

static GameState main_menu(void){
	px_inc(PX_INC1);
	px_ppu_disable(); {
		blit_palette();
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
		
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_main_menu_lz4, 1024);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	
	// Randomize the seed based on start time.
	while(true){
		for(idx = 0; idx < 255; ++idx){
			++rand_seed;
			if(JOY_START(joy_read(0))) return game_loop();
		}
		
		px_wait_nmi();
	}
	
	return game_loop();
}

static GameState pixelakes_screen(void){
	static const u8 PALETTE[] = {
		0x2D, 0x1D, 0x20, 0x06,
		0x2D, 0x1D, 0x10, 0x06,
		0x2D, 0x1D, 0x00, 0x06,
	};
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		px_addr(PAL_ADDR);
		px_blit(4, PALETTE + 0);
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_pixelakes_lz4chr, 128*16);
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_pixelakes_lz4, 1024);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	while(!JOY_START(joy_read(0))){}
	
	return main_menu();
}

static GameState debug_chr(void){
	px_ppu_disable(); {
		blit_palette();
		
		px_bg_table(1);
		px_spr_table(1);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x20), gfx_explosion_lz4chr, 32*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x80), gfx_sheet1_lz4chr, 32*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0xA0), gfx_squidman_lz4chr, 84*16);
		
		// //Top
		// px_inc(PX_INC1);
		// px_addr(NT_ADDR(0, 8, 6));
		// px_blit(16, _hextab);
		
		// // Side
		// px_inc(PX_INC32);
		// px_addr(NT_ADDR(0, 6, 8));
		// px_blit(16, _hextab);
		
		
		// // Grid
		// px_inc(PX_INC1);
		// for(iy = 0; iy < 16; ++iy){
		// 	px_addr(NT_ADDR(0, 8, 8 + iy));
		// 	for(ix = 0; ix < 16; ++ix){
		// 		PPU.vram.data = ix | 16*iy;
		// 	}
		// }
		
		px_spr_clear();
		px_wait_nmi();
	} px_ppu_enable();
	
	{
		u8 x1 = 128, x2 = 100;
		u8 y1 = 16, y2 = 220;
		u8 dx = x2 - x1;
		u8 dy = y2/2 - y1/2;
		u8 eps = 0;
		u8 inc;;
		
		if(x1 <= x2){
			inc = 1;
		} else {
			dx ^= 0xFF;
			inc = -1;
		}
		
		px_spr(x1, y1, 0x00, 0x0F);
		px_spr(x2, y2, 0x00, 'O');
		
		ix = x1;
		iy = y1;
		while(iy <= y2){
			iy += 8;
			eps += dx;
			while(eps >= dy/4) ix += inc, eps -= dy/4;
			
			px_spr(ix, iy, 0x01, 0x0E);
		}
	}
	px_spr_end();
	px_wait_nmi();
	
	debug_freeze();
}

void main(void){
	// Install the cc65 static joystick driver.
	joy_install(joy_static_stddrv);
	
	// Set an initial random seed that's not just zero.
	// The main menu increments this constantly until the player starts the game.
	rand_seed = 0x0D8E;
	
	// debug_chr();
	// game_loop();
	pixelakes_screen();
}
