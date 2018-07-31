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
	while(joy_read(0) || joy_read(1)) px_wait_nmi();
}

u8 bounce4(void){
	u8 y = (px_ticks >> 3);
	if(y & 4) y = (y ^ 0xFF);
	return (y & 3);
}

static GameState main_menu(void);
static void pause(void);
static GameState game_over(void);

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

static const u8 CHARACTER_PAL[] = {
	2, 1, 3, 2,
	2, 2, 1, 1,
	0, 0, 2, 1,
	2, 0, 1, 3,
};

static const void *CHARACTER_GFX[] = {
	gfx_squidman_lz4chr,
	gfx_azmodeus_lz4chr,
	gfx_pinkblob_lz4chr,
	gfx_blob_lz4chr,
	
	gfx_budgie_lz4chr,
	gfx_robinhood_lz4chr,
	gfx_bonecrusher_lz4chr,
	gfx_dagon_lz4chr,
	
	gfx_dog_lz4chr,
	gfx_eyeboll_lz4chr,
	gfx_kermit_lz4chr,
	gfx_ninja_lz4chr,
	
	gfx_orc_lz4chr,
	gfx_royalguard_lz4chr,
	gfx_rustknight_lz4chr,
	gfx_cyclops_lz4chr,
};

static const char *CHARACTER_BIO[] = {
	"Cthylu:\n\nGoes by \"Kathy Lu\".\nRaising capital to\nstart a childrens\nhorror series to\nhaunt dreams\nfor generations.",
	"Azmodeus:\n\nHails from the second\nlevel of Heck. The\nPrince of Greed isn't\nthe only demon that\ncan stack!",
	"Pink Blob:\n\nWith a steady career\nin treasure stacking,\nPink Blob hopes to\nprove to Green blob\nthat it's more than\njust a slimy face.",
	"Green Blob:\n\nGreen Blob is very\nshy. By becoming a\nmaster treasure\nstacker it hopes\nto gain the attention\nof Pink Blob.",
	
	"Budgie:\n\nWhen asked how much\nhis stack of treasure\ncost to aquire,\nBudgie simply repiled:\n\"Cheap! Cheap!\"",
	"Robin Hood:\n\nBy stacking the\ntreasure taken from\nthe rich, Robin Hood\nhopes to increase the\ndividends of his\ncharitable donations.",
	"Bone Crusher:\n\nBONE CRUSHER CARES\nNOT FOR TREASURE! I\nWILL CRUSH YOUR\nBONES! I WILL CRUSH\nTHEM IN A BOAT OR\nWITH A GOAT!",
	"Dagon:\n\nKnown affectionaly as\n\"Wet Willie\" by his\nclosest friends. This\ncontender is as old\nas he is fishy.",
	
	"Dog:\n\nBark! Bark! Bark!\nArf! Bark! Grrrr!\nWoof! Bark! Bark!\nRuff! Bark! Woof!",
	"Eyeboll:\n\nA keen eye for detail\nhelps achieve the\ngreatest of stacks.\nA fashionable cloak\ndoesn't hurt either.",
	"Kermit:\n\n\"Yeah, well, I've got\na dream too, but it's\nabout treasure and\nstacking and making\nmyself rich.\"",
	"Ninja:\n\nThis contender is a\nmaster of the shadow,\nand little is known\nabout his past.",
	
	"Bob:\n\nUsing its immense orc\nstrength this\ncontentder is sure to\nwin the gold. Being a\nCPA doesn't hurt\neither.",
	"Royal Guard:\n\nRoy always loves a\ngood stacking\nchallenge. He worked\nmany years as a\ntreasury guard before\nhis current post.",
	"Rust Knight:\n\noil..., *squeak*,\noil..., *squeak*,\nWD40..., *creak*",
	"Cycil:\n\nLacking depth\nperception is\nactually an advantage\nwhen playing a 2D\ngame. Use this to\nyour advantage.",
};

static const u8 CHARACTER_COUNT = sizeof(CHARACTER_PAL);

static u8 character = 0;
extern u8 character_pal;

static void character_inc(s8 amount){
	character += amount;
	if(character == 255) character = CHARACTER_COUNT - 1;
	if(character == CHARACTER_COUNT) character = 0;
}

static void load_character(void){
	decompress_lz4_to_vram(CHR_ADDR(1, 0xA0), CHARACTER_GFX[character], 84*16);
	character_pal = CHARACTER_PAL[character];
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
		grid_buffer_score(NT_ADDR(0, 17, 14));
		
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

static GameState character_select(void){
	static const u8 CURVE[] = {1, 7, 15, 26, 38, 53, 68, 84, 99, 114, 129, 141, 152, 160, 166, 168};
	
	u16 bio_addr = NT_ADDR(0, 7, 11);
	register const char *bio_cursor = CHARACTER_BIO[character];
	
	px_inc(PX_INC1);
	px_ppu_disable(); {
		blit_palette(CLR_BLACK);
		
		px_bg_table(0);
		decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_neschar_lz4chr, 128*16);
		
		px_spr_table(1);
		load_character();
		
		decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_character_select_lz4, 1024);
		
		px_addr(bio_addr);
		for(ix = 0; bio_cursor[ix]; ++ix){
			if(bio_cursor[ix] == '\n'){
				bio_addr += 32;
				px_addr(bio_addr);
			} else {
				PPU.vram.data = bio_cursor[ix];
			}
		}
		
		px_addr(AT_ADDR(0));
		px_fill(64, 0x55);
		
		px_buffer_set_color(0, CLR_BG);
		
		PX.scroll_y = 168;
		px_spr_clear();
		px_wait_nmi();
	} px_ppu_enable();
	
	for(idx = 0; idx < sizeof(CURVE)/2; ++idx){
		PX.scroll_y = 480 + 168 - 2*CURVE[idx];
		px_wait_nmi();
	}
	
	PX.scroll_y = 0;
	wait_noinput();
	
	while(true){
		joy0 = joy_read(0);
		if(JOY_START(joy0)) return game_loop();
		if(JOY_DOWN(joy0)){character_inc(1); break;}
		if(JOY_UP(joy0)){character_inc(-1); break;}
		
		idx = ((px_ticks >> 2) & 0x6) + 17;
		player_sprite(36, 128, idx);
		
		px_spr_end();
		px_wait_nmi();
	}
	
	px_spr_clear();
	for(idx = sizeof(CURVE)/2; idx < sizeof(CURVE); ++idx){
		PX.scroll_y = 480 + 168 - 2*CURVE[idx];
		px_wait_nmi();
	}
	
	return character_select();
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
	
	// Black out the second screen.
	px_addr(NT_ADDR(2, 0, 0));
	px_fill(32*30, 0x20);
	
	// debug_chr();
	// main_menu();
	// character_select();
	// game_loop();
	pixelakes_screen();
}
