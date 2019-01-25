#include <nes.h>

#include "pixler/pixler.h"
#include "shared.h"
#include "gfx/gfx.h"

#define CLR_BG 0x0C

static const u8 PALETTE[] = {
	CLR_BG, 0x21, 0x20, 0x28, // DROPS0: dark orange, green, purple
	CLR_BG, 0x17, 0x28, 0x20, // DROPS1: orange, yellow, white
	CLR_BG, 0x11, 0x22, 0x32, // BG: blue, light blue, dark cyan
	CLR_BG, 0x28, 0x38, 0x20, // FG
	
	CLR_BG, 0x07, 0x1A, 0x14, // DROPS0
	CLR_BG, 0x17, 0x28, 0x20, // DROPS1
	CLR_BG, 0x11, 0x22, 0x32, // BG
	CLR_BG, 0x08, 0x18, 0x28, // PLAYER
};

static GameState main_menu(void);
static void pause(void);
static GameState game_over(void);

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
	
	px_ppu_enable();
	
	music_play(0);

	while(true){
		px_spr_end();
		px_wait_nmi();
	}
}
