#include <nes.h>

#include "pixler/pixler.h"
#include "shared.h"
#include "gfx/gfx.h"

#define CLR_BG 0x0C

static const u8 PALETTE[] = {
	CLR_BG, 0x21, 0x20, 0x28,
	CLR_BG, 0x00, 0x00, 0x00,
	CLR_BG, 0x00, 0x00, 0x00,
	CLR_BG, 0x00, 0x00, 0x00,
	
	CLR_BG, 0x16, 0x1A, 0x14,
	CLR_BG, 0x00, 0x00, 0x00,
	CLR_BG, 0x00, 0x00, 0x00,
	CLR_BG, 0x00, 0x00, 0x00,
};

static GameState main_menu(void);
static void pause(void);
static GameState game_over(void);

// n = 256; [int(round(16*math.sin(2.0*math.pi*i/float(n)))) for i in range(n)]
static const s8 BOB[] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -4, -5, -5, -5, -6, -6, -6, -7, -7, -8, -8, -8, -9, -9, -9, -10, -10, -10, -10, -11, -11, -11, -12, -12, -12, -12, -13, -13, -13, -13, -14, -14, -14, -14, -14, -14, -15, -15, -15, -15, -15, -15, -15, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -15, -15, -15, -15, -15, -15, -15, -14, -14, -14, -14, -14, -14, -13, -13, -13, -13, -12, -12, -12, -12, -11, -11, -11, -10, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -2, -2, -2, -1, -1, 0};

/*
path_x = [80,  62,  56,  63,  80,  99, 110, 120, 129, 139, 149, 161, 174, 187, 192, 184, 168]
path_y = [117, 112,  96,  79,  75,  79,  93, 104, 116, 128, 137, 144, 147, 144, 132, 118, 114]
n = 16; [(n - (t&(n-1)))*path_x[n - t/n]/n + (t&(n-1))*path_x[n - 1 - t/n]/n for t in range(256)]
*/
u8 PATH_X[] = {168, 168, 170, 170, 172, 172, 174, 174, 176, 176, 178, 178, 180, 180, 182, 182, 184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 190, 191, 191, 192, 191, 191, 191, 190, 190, 190, 189, 189, 189, 188, 188, 188, 187, 187, 187, 187, 185, 184, 183, 183, 182, 181, 181, 180, 178, 178, 177, 176, 176, 175, 174, 174, 173, 172, 171, 170, 169, 168, 167, 167, 166, 165, 164, 163, 162, 161, 160, 161, 159, 158, 157, 157, 156, 155, 155, 154, 153, 153, 152, 151, 151, 150, 149, 149, 147, 147, 147, 145, 145, 145, 143, 143, 143, 141, 141, 141, 139, 139, 139, 139, 138, 137, 136, 136, 135, 134, 134, 133, 132, 132, 131, 130, 130, 129, 128, 129, 127, 127, 126, 126, 125, 125, 124, 124, 123, 123, 122, 122, 121, 121, 120, 120, 118, 118, 117, 117, 116, 116, 115, 115, 113, 113, 112, 112, 111, 111, 110, 110, 109, 108, 107, 106, 105, 105, 104, 104, 103, 102, 102, 101, 100, 99, 98, 99, 97, 96, 95, 94, 93, 91, 90, 89, 88, 87, 85, 84, 83, 82, 81, 80, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56, 56, 55, 56, 55, 56, 56, 57, 57, 58, 58, 59, 58, 59, 59, 60, 60, 61, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 72, 73, 74, 75, 76, 77, 78};
u8 PATH_Y[] = {114, 113, 113, 114, 114, 114, 115, 115, 116, 115, 115, 116, 116, 116, 117, 117, 118, 118, 119, 119, 121, 122, 122, 123, 125, 125, 126, 126, 128, 129, 129, 130, 132, 132, 133, 134, 135, 135, 136, 137, 138, 138, 139, 140, 141, 141, 142, 143, 144, 144, 144, 144, 144, 144, 145, 145, 145, 145, 145, 146, 146, 146, 146, 146, 147, 146, 146, 146, 146, 146, 145, 145, 145, 145, 145, 144, 144, 144, 144, 144, 144, 143, 143, 142, 142, 141, 141, 140, 140, 140, 139, 139, 138, 138, 137, 137, 137, 136, 135, 135, 134, 134, 133, 133, 132, 131, 131, 130, 130, 129, 129, 128, 128, 127, 126, 125, 125, 124, 123, 122, 122, 121, 120, 119, 119, 118, 117, 116, 116, 114, 114, 113, 113, 111, 111, 110, 110, 108, 108, 107, 107, 105, 105, 104, 104, 102, 102, 101, 101, 100, 99, 98, 98, 97, 97, 95, 95, 94, 94, 93, 93, 91, 90, 89, 88, 87, 87, 86, 85, 84, 83, 83, 82, 81, 80, 79, 79, 78, 78, 78, 77, 77, 77, 76, 76, 76, 75, 75, 75, 74, 74, 74, 75, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77, 77, 78, 78, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 116, 116, 116};


void main(void){
	// Set the palette.
	waitvsync();
	px_addr(PAL_ADDR);
	px_blit(32, PALETTE);
	
	// Load CHR.
	px_spr_table(0);
	px_bg_table(0);
	decompress_lz4_to_vram(CHR_ADDR(0, 0x00), gfx_CHR0_lz4chr);
	
	music_init(MUSIC);
	// sound_init(SOUNDS);
	
	px_inc(PX_INC1);
	decompress_lz4_to_vram(NT_ADDR(0, 0, 0), gfx_splash_lz4);
	decompress_lz4_to_vram(NT_ADDR(2, 0, 0), gfx_splash_lz4);
	
	px_ppu_enable();
	music_play(0);

	while(true){
		static s8 offset;
		
		offset = BOB[px_ticks & (sizeof(BOB) - 1)];
		PX.scroll_y = 480 + offset;
		
		// Blocker sprites.
		px_spr( 80, 116 - offset, 0x20, 0xE0);
		px_spr(168, 113 - offset, 0x20, 0xE0);
		
		// DEBUG_PROFILE_START();
		px_spr(PATH_X[(px_ticks - 0x00) & 0xFF], PATH_Y[(px_ticks - 0x00) & 0xFF] - offset, 0x00, 0xF0); // G
		px_spr(PATH_X[(px_ticks - 0x08) & 0xFF], PATH_Y[(px_ticks - 0x08) & 0xFF] - offset, 0x00, 0xF1); // l
		px_spr(PATH_X[(px_ticks - 0x10) & 0xFF], PATH_Y[(px_ticks - 0x10) & 0xFF] - offset, 0x00, 0xF2); // o
		px_spr(PATH_X[(px_ticks - 0x18) & 0xFF], PATH_Y[(px_ticks - 0x18) & 0xFF] - offset, 0x00, 0xF3); // b
		px_spr(PATH_X[(px_ticks - 0x20) & 0xFF], PATH_Y[(px_ticks - 0x20) & 0xFF] - offset, 0x00, 0xF4); // a
		px_spr(PATH_X[(px_ticks - 0x28) & 0xFF], PATH_Y[(px_ticks - 0x28) & 0xFF] - offset, 0x00, 0xF1); // l
		
		px_spr(PATH_X[(px_ticks - 0x38) & 0xFF], PATH_Y[(px_ticks - 0x38) & 0xFF] - offset, 0x00, 0xF0); // G
		px_spr(PATH_X[(px_ticks - 0x40) & 0xFF], PATH_Y[(px_ticks - 0x40) & 0xFF] - offset, 0x00, 0xF4); // a
		px_spr(PATH_X[(px_ticks - 0x48) & 0xFF], PATH_Y[(px_ticks - 0x48) & 0xFF] - offset, 0x00, 0xF5); // m
		px_spr(PATH_X[(px_ticks - 0x50) & 0xFF], PATH_Y[(px_ticks - 0x50) & 0xFF] - offset, 0x00, 0xF6); // e
		
		px_spr(PATH_X[(px_ticks - 0x60) & 0xFF], PATH_Y[(px_ticks - 0x60) & 0xFF] - offset, 0x00, 0xF7); // J
		px_spr(PATH_X[(px_ticks - 0x68) & 0xFF], PATH_Y[(px_ticks - 0x68) & 0xFF] - offset, 0x00, 0xF4); // a
		px_spr(PATH_X[(px_ticks - 0x70) & 0xFF], PATH_Y[(px_ticks - 0x70) & 0xFF] - offset, 0x00, 0xF5); // m
		
		px_spr(PATH_X[(px_ticks - 0x80) & 0xFF], PATH_Y[(px_ticks - 0x80) & 0xFF] - offset, 0x00, 0xF8); // 2
		px_spr(PATH_X[(px_ticks - 0x88) & 0xFF], PATH_Y[(px_ticks - 0x88) & 0xFF] - offset, 0x00, 0xF9); // 0
		px_spr(PATH_X[(px_ticks - 0x90) & 0xFF], PATH_Y[(px_ticks - 0x90) & 0xFF] - offset, 0x00, 0xFA); // 1
		px_spr(PATH_X[(px_ticks - 0x98) & 0xFF], PATH_Y[(px_ticks - 0x98) & 0xFF] - offset, 0x00, 0xFB); // 9
		// DEBUG_PROFILE_END();
		
		px_spr_end();
		px_wait_nmi();
	}
}
