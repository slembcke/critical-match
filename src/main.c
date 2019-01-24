#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"
#include "gfx/gfx.h"

u8 joy0, joy1;
static u16 timeout;

#define CLR_BG 0x02

static const u8 PALETTE[] = {
	CLR_BG, 0x07, 0x1A, 0x14, // DROPS0: dark orange, green, purple
	CLR_BG, 0x17, 0x28, 0x20, // DROPS1: orange, yellow, white
	CLR_BG, 0x11, 0x22, 0x32, // BG: blue, light blue, dark cyan
	CLR_BG, 0x28, 0x38, 0x20, // FG
	
	CLR_BG, 0x07, 0x1A, 0x14, // DROPS0
	CLR_BG, 0x17, 0x28, 0x20, // DROPS1
	CLR_BG, 0x11, 0x22, 0x32, // BG
	CLR_BG, 0x08, 0x18, 0x28, // PLAYER
};

static void wait_noinput(void){
	while(joy_read(0) || joy_read(1)) px_wait_nmi();
}

u8 bounce4(void){
	u8 y = (px_ticks >> 3);
	if(y & 4) y = (y ^ 0xFF);
	return (y & 3);
}

static void px_ppu_sync_off(void){
	px_mask &= ~PX_MASK_RENDER_ENABLE;
	px_wait_nmi();
}

static void px_ppu_sync_on(void){
	px_mask |= PX_MASK_RENDER_ENABLE;
	px_wait_nmi();
}

static GameState main_menu(void);
static void pause(void);
static GameState game_over(void);

static GameState game_loop(bool tutorial){
	music_stop();
	
	player_init();
	grid_init(tutorial);
	
	px_inc(PX_INC1);
	px_ppu_sync_off(); {
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_board_lz4);
		
		px_spr_clear();
	} px_ppu_sync_on();
	
	wait_noinput();
	
	// music_play(0);
	
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
		
		player_draw_grapple();
		
		px_spr_end();
		DEBUG_PROFILE_END();
		px_wait_nmi();
	}
	
	px_wait_nmi();
	if(tutorial){
		return main_menu();
	} else {
		return game_over();
	}
}

static void pause(void){
	wait_noinput();
	while(!JOY_START(joy_read(0))){}
	wait_noinput();
}

static GameState final_score(s16 scroll_v){
	u16 timeout;
	u16 scroll_y = 0;
	
	px_buffer_inc(PX_INC1);
	px_ppu_sync_off(); {
		px_addr(NT_ADDR(0, 10, 12));
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_game_over_lz4);
		
		// Score
		grid_buffer_score(NT_ADDR(0, 17, 14));
		
		px_spr_clear();
	} px_ppu_sync_on();
	
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
	
	// Wait a while or until start is pressed.
	for(timeout = 30*60; timeout > 0; --timeout){
		if(JOY_START(joy_read(0))) break;
		px_wait_nmi();
	}
	
	exit(0);
}

// TODO This is pretty terrible.
static GameState game_over(void){
	static const u8 BOOM[] = {
		69, 50, 61, 52, 78, 85, 38, 19, 46, 44, 65, 27,
		51, 58, 68, 37, 26, 12, 11, 57, 41, 22, 73, 77,
		84, 82, 59, 36, 34, 29, 18, 21, 45, 83,  9, 13,
		74, 53, 33, 66, 35, 67, 62, 25, 14, 49, 70, 86,
		42, 60, 43, 10, 17, 28, 81, 76, 20, 54, 30, 75,
	};
	
	u16 scroll_y = 0, scroll_v = 0;
	u8 spr_y, ticks = 0;
	
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

static GameState credits_screen(void){
	timeout = 0;
	
	px_ppu_sync_off(); {
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_credits_lz4);
		
		px_spr_clear();
	} px_ppu_sync_on();
	
	wait_noinput();
	for(timeout = 30*60; timeout > 0; --timeout){
		if(joy_read(0) != 0) break;
		px_wait_nmi();
	}
	
	wait_noinput();
	return main_menu();
}

static const s8 COS_32[] = {
	12, 11, 11, 9, 8, 6, 4, 2, 0, -2, -4, -6, -8, -9, -11, -11,
	-12, -11, -11, -9, -8, -6, -4, -2, 0, 2, 4, 6, 8, 9, 11, 11,
};

static void draw_orbit(void){
	if(idx){
		px_spr(44 + ix, 183 + iy, 0x00, 0x8D);
	} else {
		px_spr(44 - ix, 183 + iy, 0x00, 0x8C);
	}
}

static void draw_orbit2(void){
	ix += COS_32[idx + 0 & 31];
	iy += (COS_32[idx + 4 & 31] >> 1);
	px_spr(ix + 4, iy + 3, idx & 16 ? 0x20 : 0x00, 0x8D);
}

static GameState main_menu(void){
	static u8 menu_item;
	static u8 joy_prev;
	
	timeout = 0;
	menu_item = 0;
	
	music_stop();
	
	px_inc(PX_INC1);
	px_ppu_sync_off(); {
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_menu_lz4);
	} px_ppu_sync_on();
	
	// music_play(0);

	wait_noinput();
	
	// Randomize the seed based on start time.
	for(timeout = 30*60; timeout > 0; --timeout){
		for(idx = 0; idx < 60; ++idx){
			++rand_seed;
			if(JOY_START(joy_read(0)) && menu_item == 0){
				sound_play(SOUND_MATCH);
				return game_loop(false);
			}
		}
		
		joy_prev = joy0;
		joy0 = joy_read(0);
		idx = (joy0 ^ joy_prev) & joy0;
		
		if(JOY_DOWN(idx) && menu_item < 2){
			sound_play(SOUND_PICKUP);
			++menu_item;
		}
		
		if(JOY_UP(idx) && menu_item > 0){
			sound_play(SOUND_PICKUP);
			--menu_item;
		}
		
		if(JOY_START(joy0)){
			if(menu_item == 1){
				sound_play(SOUND_MATCH);
				game_loop(true);
			}
			if(menu_item == 2){
				sound_play(SOUND_MATCH);
				credits_screen();
			}
		}
		
		// Draw atoms.
		ix = 2*8, iy = 2*8;
		idx = px_ticks + 12;
		draw_orbit2();
		
		ix = 8*8, iy = 3*8;
		idx = px_ticks + 0;
		draw_orbit2();
		
		ix = 12*8, iy = 2*8;
		idx = px_ticks + 14;
		draw_orbit2();
		
		ix = 20*8, iy = 4*8;
		idx = px_ticks + 6;
		draw_orbit2();
		
		ix = 25*8, iy = 2*8;
		idx = px_ticks + 10;
		draw_orbit2();
		
		ix = 3*8, iy = 12*8;
		idx = px_ticks + 2;
		draw_orbit2();
		
		ix = 27*8, iy = 12*8;
		idx = px_ticks + 8;
		draw_orbit2();
		
		// Draw cursor/atom.
		idx = (px_ticks & 16) == 0;
		ix = COS_32[px_ticks + 0 & 31];
		iy = (COS_32[px_ticks + 4 & 31] >> 1) + 16*menu_item;
		
		draw_orbit();
		block_sprite(40, 179 + 16*menu_item, BLOCK_CHEST | BLOCK_COLOR_YELLOW);
		idx = !idx;
		draw_orbit();
		
		ix = px_ticks & 127;
		idx = ((ix >> 1) & 14) + 1;
		if(px_ticks & 128){
			ix = (ix ^ 0xFF) & 0x7F;
			idx -= 1;
		}
		player_sprite(64 + ix, 153, idx);
		
		px_spr_end();
		px_wait_nmi();
	}
	
	return game_loop(false);
}

/*
#ifdef DEBUG
static GameState debug_chr(void){
	px_ppu_sync_off(); {
		// blit_palette();
		
		px_bg_table(1);
		px_spr_table(1);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x00), gfx_neschar_lz4chr);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x20), gfx_explosion_lz4chr);
		decompress_lz4_to_vram(CHR_ADDR(1, 0x80), gfx_sheet1_lz4chr);
		// decompress_lz4_to_vram(CHR_ADDR(1, 0xA0), gfx_squidman_lz4chr);
		
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
	} px_ppu_sync_on();
	
	
	debug_freeze();
}
#endif
*/

void main(void){
	px_bank_select(0);
	
	// Install the cc65 static joystick driver.
	joy_install(joy_static_stddrv);
	
	// Set an initial random seed that's not just zero.
	// The main menu increments this constantly until the player starts the game.
	rand_seed = 0x0D8E;
	
	// Set the palette.
	waitvsync();
	px_addr(PAL_ADDR);
	px_blit(32, PALETTE);
	
	// Load BG tiles..
	px_bg_table(0);
	
	// Load sprites.
	px_spr_table(1);
	
	// Black out the second screen.
	px_addr(NT_ADDR(2, 0, 0));
	px_fill(32*30, 0x00);
	
	music_init(MUSIC);
	sound_init(SOUNDS);
	
	// debug_chr();
	main_menu();
}
