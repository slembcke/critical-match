#include <stdlib.h>
#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler.h"
#include "shared.h"

GameState Freeze(){
	px_wait_nmi();
	return Freeze();
}

GameState debug_display(void){
	PPU.mask = 0x1E;
	return Freeze();
}

void player_right_sprite(u8 x, u8 y, u8 frame);
void player_left_sprite(u8 x, u8 y, u8 frame);

GameState debug_palette(){
	register u8 pal = 0;
	
	// Enable rendering.
	PPU.mask = 0x1E;
	px_wait_nmi();
	
	idx = 0;
	while(true){
		joy0 = joy_read(0);
		
		if(JOY_START(joy0)) break;
		
		if(JOY_LEFT(joy0)) pal = (pal - 1) & 3;
		if(JOY_RIGHT(joy0)) pal = (pal + 1) & 3;
		
		px_inc(PX_INC1);
		px_buffer_data(4, PAL_ADDR);
		memcpy(PX.buffer, PALETTE + 4*pal, 4);
		
		// Wait until button up.
		while(joy_read(0)) px_wait_nmi();
		px_wait_nmi();
	}
	
	PPU.mask = 0x0;
	return board();
}

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(x, min, max) MAX(min, MIN(x, max))

#define PLAYER_MAX_SPEED 0x0200
#define PLAYER_ACCEL 0x0040

struct Player {
	u16 pos_x;
	s16 vel_x;
	
	// Desired movement.
	s16 move;
	
	bool facingRight;
	bool grounded;
} Player;

GameState debug_player(){
	register u8 ticks = 0;
	struct Player player = {0x8000};
	
	// Enable rendering.
	PPU.mask = 0x1E;
	px_wait_nmi();
	
	while(true){
		joy0 = joy_read(0);
		
		player.move = 0;
		if(JOY_LEFT(joy0)) player.move -= PLAYER_MAX_SPEED;
		if(JOY_RIGHT(joy0)) player.move += PLAYER_MAX_SPEED;
		
		player.vel_x += CLAMP(player.move - player.vel_x, -PLAYER_ACCEL, PLAYER_ACCEL);
		player.pos_x += player.vel_x;
		
		// Update the facing direction.
		if(player.vel_x > 0){
			player.facingRight = true;
		} else if(player.vel_x < 0){
			player.facingRight = false;
		}
		
		ix = player.pos_x >> 8;
		if((player.vel_x >> 8) == 0){
			// Idle
			if(player.facingRight){
				player_right_sprite(ix, 220, ((ticks >> 3) & 3) + 8);
			} else {
				player_left_sprite(ix, 220, ((ticks >> 3) & 3) + 8);
			}
		} else {
			// Run
			if(player.facingRight){
				player_right_sprite(ix, 220, (ix >> 2) & 7);
			} else {
				player_left_sprite(ix, 220, (ix >> 2) & 7);
			}
		}
		
		++ticks;
		px_wait_nmi();
	}
	
	exit(0);
	// return Freeze();
}

GameState debug_chr(void){
	static const char HEX[] = "0123456789ABCDEF";
	
	// Top
	px_inc(PX_INC1);
	px_addr(NT_ADDR(0, 8, 6));
	px_blit(16, HEX);
	
	// Side
	px_inc(PX_INC32);
	px_addr(NT_ADDR(0, 6, 8));
	px_blit(16, HEX);
	
	// Grid
	px_inc(PX_INC1);
	for(iy = 0; iy < 16; ++iy){
		px_addr(NT_ADDR(0, 8, 8 + iy));
		for(ix = 0; ix < 16; ++ix){
			PPU.vram.data = ix | 16*iy;
		}
	}
	
	return debug_player();
}
