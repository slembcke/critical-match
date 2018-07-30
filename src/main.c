#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"
#include "gfx/gfx.h"

u8 joy0, joy1;

#define CLR_BG 0x2D
#define CLR_BLACK 0x1D
#define CLR_YELLOW 0x28
#define CLR_1 0x02 // blue
#define CLR_2 0x07 // red
#define CLR_3 0x1B // green
#define CLR_4 0x14 // purple

static void wait_noinput(void){
	while(joy_read(0) || joy_read(1)){}
}

u8 bounce4(void){
	u8 y = (px_ticks >> 3);
	if(y & 4) y = (y ^ 0xFF);
	return (y & 3);
}

static GameState main_menu(void);
static void pause(void);
static GameState game_over(void);

static void blank_screen2(){
	u16 addr = NT_ADDR(2, 0, 0);
	for(iy = 0; iy < 15; ++iy){
		px_buffer_data(64, addr);
		memset(PX.buffer, 0x20, 64);
		addr += 64;
		px_wait_nmi();
	}
}

static void blit_palette(u8 bg_color){
	static const u8 PALETTE[] = {
		CLR_BG, CLR_BLACK, CLR_YELLOW, CLR_1,
		CLR_BG, CLR_BLACK, CLR_YELLOW, CLR_2,
		CLR_BG, CLR_BLACK, CLR_YELLOW, CLR_3,
		CLR_BG, CLR_BLACK, CLR_YELLOW, CLR_4,
	};
	
	px_addr(PAL_ADDR);
	px_blit(sizeof(PALETTE), PALETTE);
	px_blit(sizeof(PALETTE), PALETTE);
	
	px_addr(PAL_ADDR);
	PPU.vram.data = bg_color;
}

static void load_character(void){
	decompress_lz4_to_vram(CHR_ADDR(1, 0xA0), gfx_bonecrusher_lz4chr, 84*16);
}

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
	
	// for(idx = 8; idx < GRID_BYTES - 8; ++idx){
	// 	GRID[idx] = BLOCK_GARBAGE;
	// }
	// GRID[GRID_BYTES - 11] = BLOCK_EMPTY;
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		blit_palette(CLR_BLACK);
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x80), gfx_sheet1_lz4chr, 128*16);
		
		px_spr_table(1);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x20), gfx_explosion_lz4chr, 32*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x80), gfx_sheet1_lz4chr, 128*16);
		load_character();
		
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_board_lz4, 1024);
		
		px_addr(AT_ADDR(0));
		px_fill(64, 0x55);
		
		px_buffer_set_color(0, CLR_BG);
		
		px_spr_clear();
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

static GameState final_score(s16 scroll_v){
	u16 scroll_y = 0;
	
	px_buffer_inc(PX_INC1);
	px_ppu_disable(); {
		px_addr(PAL_ADDR);
		PPU.vram.data = CLR_BLACK;
		
		px_addr(NT_ADDR(0, 10, 12));
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_game_over_lz4, 1024);
		
		px_addr(AT_ADDR(0));
		px_fill(64, 0x55);
		
		// Score
		px_buffer_data(5, NT_ADDR(0, 17, 14));
		memset(PX.buffer, 0, 5);
		ultoa(grid_get_score(), PX.buffer, 10);
		
		px_buffer_set_color(0, CLR_BG);
		
		px_spr_clear();
		px_wait_nmi();
	} px_ppu_enable();
	
	for(ix = 0; ix < 240; ++ix){
		scroll_v += 16;
		scroll_y += scroll_v;
		
		if(scroll_y > (240 << 8)){
			scroll_v = -scroll_v/2;
			scroll_y = (240 << 8);
		}
		
		PX.scroll_y = 240 - (scroll_y >> 8);
		px_wait_nmi();
	}
	
	PX.scroll_y = 0;
	px_wait_nmi();
	
	wait_noinput();
	
	// Wait until start is pressed.
	while(!JOY_START(joy_read(0))){}
	
	return main_menu();
}

// TODO This is pretty terrible.
static GameState game_over(void){
	u8 BOOM[] = {
		69, 50, 61, 52, 78, 85, 38, 19, 46, 44, 65, 27,
		51, 58, 68, 37, 26, 12, 11, 57, 41, 22, 73, 77,
		84, 82, 59, 36, 34, 29, 18, 21, 45, 83,  9, 13,
		74, 53, 33, 66, 35, 67, 62, 25, 14, 49, 70, 86,
		42, 60, 43, 10, 17, 28, 81, 76, 20, 54, 30, 75,
	};
	
	u16 scroll_y = 0, scroll_v = 0;
	u8 spr_y, ticks = 0;
	
	blank_screen2();
	
	while(scroll_y < (240 << 8)){
		scroll_v += 2;
		scroll_y += scroll_v;
		
		if(ticks < sizeof(BOOM)){
			idx = BOOM[ticks];
			grid_set_block(idx, BLOCK_EMPTY);
			
			for(ix = 0; ix < 8 && ix < ticks; ++ix){
				idx = BOOM[ticks - ix];
				spr_y = grid_block_y(idx, -6) - 8*ix;
				if(spr_y + (scroll_y >> 8) < 240) explosion_sprite(grid_block_x(idx, 0), spr_y + (scroll_y >> 8), ix);
			}
			
			px_spr_end();
		} else {
			px_spr_clear();
		}
		
		PX.scroll_y = 480 - (scroll_y >> 8);
		px_wait_nmi();
		
		if((px_ticks & 3) == 0) ++ticks;
	}
	
	PX.scroll_y = 240;
	px_wait_nmi();
	
	return final_score(scroll_v);
}

static const char *TEXT = "Cathylu:\n\nGoes by \"Katie Lu\".\nRaising capital to\nstart a childrens\nhorror series to\nhaunt generations\nof dreams.";

static void blit_text(void){
	u16 addr = NT_ADDR(0, 7, 11);
	register const char *cursor = TEXT;
	
	px_addr(addr);
	for(ix = 0; cursor[ix]; ++ix){
		if(cursor[ix] == '\n'){
			addr += 32;
			px_addr(addr);
		} else {
			PPU.vram.data = cursor[ix];
		}
	}
}

static GameState character_select(void){
	px_inc(PX_INC1);
	px_ppu_disable(); {
		blit_palette(CLR_BLACK);
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
		
		px_spr_table(1);
		load_character();
		
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_character_select_lz4, 1024);
		
		blit_text();
		
		px_addr(AT_ADDR(0));
		px_fill(64, 0x55);
		
		px_buffer_set_color(0, CLR_BG);
		
		px_spr_clear();
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	
	while(true){
		if(JOY_START(joy_read(0))) break;
		
		idx = ((px_ticks >> 2) & 0x6) + 17;
		player_sprite(36, 132, idx);
		
		px_spr_end();
		px_wait_nmi();
	}
	
	return game_loop();
}

static GameState main_menu(void){
	px_inc(PX_INC1);
	px_ppu_disable(); {
		static const u8 PALETTE[] = {
			0x1D, 0x2D, 0x3D, 0x11,
			0x1D, 0x18, 0x28, 0x38,
			0x1D, 0x09, 0x09, 0x09,
			0x1D, 0x01, 0x01, 0x01,
			0x1D, 0x2D, 0x27, 0x20,
			0x1D, 0x2D, 0x2C, 0x20,
			0x1D, 0x1D, 0x28, 0x1A,
			0x1D, 0x1D, 0x28, 0x13,
		};
		
		static const u8 ATTR[] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00,
			0x00, 0x55, 0x55, 0x55, 0x00, 0x55, 0x55, 0x55,
			0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};
		
		px_addr(PAL_ADDR);
		px_blit(sizeof(PALETTE), PALETTE);
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_menu_tiles_lz4chr, 128*16);
		
		px_spr_table(1);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x80), gfx_logo64_lz4chr, 32*16);
		
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_main_menu_lz4, 32*30);
		px_addr(AT_ADDR(0));
		px_blit(sizeof(ATTR), ATTR);
		
		px_wait_nmi();
	} px_ppu_enable();
	
	wait_noinput();
	
	// Randomize the seed based on start time.
	while(true){
		for(idx = 0; idx < 60; ++idx){
			++rand_seed;
			if(JOY_START(joy_read(0))) return character_select();
		}
		
		for(iy = 0; iy < 4; ++iy){
			for(ix = 0; ix < 8; ++ix){
				idx = 0x80 + 8*iy + ix;
				px_spr(16 + 8*ix, 240 - 46 + 8*iy + bounce4(), idx < 0x92 ? 0x00 : 0x01, idx);
			}
		}
		
		px_wait_nmi();
	}
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

#ifdef DEBUG
static GameState debug_chr(void){
	px_ppu_disable(); {
		blit_palette();
		
		px_bg_table(1);
		px_spr_table(1);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x00), gfx_neschar_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x20), gfx_explosion_lz4chr, 32*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x80), gfx_sheet1_lz4chr, 128*16);
		decompress_lz4_to_vram(CHR_ADDR(1, 0xA0), gfx_squidman_lz4chr, 84*16);
		
		//Top
		px_inc(PX_INC1);
		px_addr(NT_ADDR(0, 8, 6));
		px_blit(16, _hextab);
		
		// Side
		px_inc(PX_INC32);
		px_addr(NT_ADDR(0, 6, 8));
		px_blit(16, _hextab);
		
		
		// Grid
		px_inc(PX_INC1);
		for(iy = 0; iy < 16; ++iy){
			px_addr(NT_ADDR(0, 8, 8 + iy));
			for(ix = 0; ix < 16; ++ix){
				PPU.vram.data = ix | 16*iy;
			}
		}
		
		px_spr_clear();
		px_wait_nmi();
	} px_ppu_enable();
	
	
	debug_freeze();
}
#endif

void main(void){
	px_bank_select(0);
	
	// Install the cc65 static joystick driver.
	joy_install(joy_static_stddrv);
	
	// Set an initial random seed that's not just zero.
	// The main menu increments this constantly until the player starts the game.
	rand_seed = 0x0D8E;
	
	// debug_chr();
	// main_menu();
	character_select();
	// game_loop();
	pixelakes_screen();
}
