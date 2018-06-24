#include <nes.h>
#include <joystick.h>

#include "pixler.h"
#include "shared.h"

#define PLAYER_MAX_SPEED 0x0300
#define PLAYER_ACCEL 0x0060
#define PLAYER_GRAVITY 0x00C0
#define PLAYER_MAX_FALL (5*PLAYER_MAX_SPEED/2)
#define PLAYER_JUMP 0x0500
#define PLAYER_JUMP_TICKS 5

void player_sprite(u8 x, u8 y, u8 frame);
void cursor_sprite(u8 x, u8 y);

typedef struct {
	u16 pos_x, pos_y;
	s16 vel_x, vel_y;
	
	// Desired movement.
	s16 move;
	
	bool facingRight;
	bool grounded;
	
	// Remaining ticks of jump power.
	u8 jump_ticks;
} Player;

static const Player INIT = {64 << 8, 16 << 8, 0, 0};
static Player player = {};

void player_init(void){
	player = INIT;
	player.facingRight = true;
}

#define grid_block_at(ix, iy) (((iy >> 1) & 0xF8) | ((ix >> 4)))
static u16 bx, by, block;

void player_tick(u8 joy){
	player.move = 0;
	if(JOY_LEFT(joy)) player.move -= PLAYER_MAX_SPEED;
	if(JOY_RIGHT(joy)) player.move += PLAYER_MAX_SPEED;
	
	player.pos_x += player.vel_x;
	player.pos_y += player.vel_y;
	
	player.vel_x += CLAMP(player.move - player.vel_x, -PLAYER_ACCEL, PLAYER_ACCEL);
	player.vel_y = MAX(-PLAYER_MAX_FALL, player.vel_y - PLAYER_GRAVITY);
	
	if(JOY_BTN_1(joy) && player.jump_ticks > 0){
		player.vel_y = PLAYER_JUMP;
		--player.jump_ticks;
	}
	
	// Ground detection.
	player.grounded = false;
	
	ix = (player.pos_x >> 8);
	iy = (player.pos_y >> 8) - 1;
	idx = grid_block_at(ix, iy);
	block = GRID[idx];
	by = ((iy << 8) & 0xF000) + 0x1000;
	if(block && player.pos_y <= by){
		player.pos_y = by;
		player.vel_y = MAX(0, player.vel_y);
		
		player.grounded = true;
		if(!JOY_BTN_1(joy)) player.jump_ticks = PLAYER_JUMP_TICKS;
	}
	
	// Head collision.
	ix = (player.pos_x >> 8);
	iy = (player.pos_y >> 8) + 16;
	idx = grid_block_at(ix, iy);
	block = GRID[idx];
	by = ((iy << 8) & 0xF000);
	if(block && player.pos_y <= by){
		// player.pos_y = by;
		player.vel_y = MIN(0, player.vel_y);
	}
	
	// Left collision.
	ix = (player.pos_x >> 8) - 5;
	iy = (player.pos_y >> 8) + 4;
	idx = grid_block_at(ix, iy);
	block = GRID[idx];
	bx = ((ix << 8) & 0xF000) + 0x1000;
	if(block && player.pos_x <= bx + 1024){
		player.pos_x = bx + 1024;
		player.vel_x = MAX(0, player.vel_x);
	}
	
	// Right collision.
	ix = (player.pos_x >> 8) + 5;
	iy = (player.pos_y >> 8) + 4;
	idx = grid_block_at(ix, iy);
	block = GRID[idx];
	bx = ((ix << 8) & 0xF000);
	if(block && player.pos_x >= bx - 1024){
		player.pos_x = bx - 1024;
		player.vel_x = MIN(0, player.vel_x);
	}
	
	// Update the facing direction.
	if(player.vel_x > 0){
		player.facingRight = true;
	} else if(player.vel_x < 0){
		player.facingRight = false;
	}
	
	ix = player.pos_x >> 8;
	iy = player.pos_y >> 8;
	if(player.grounded){
		if((player.vel_x >> 8) == 0){
			// Idle
			idx = ((px_ticks >> 2) & 0x6) + 16 + player.facingRight;
		} else {
			// Run
			idx =((ix >> 1) & 14) + player.facingRight;
		}
	} else {
		if(player.vel_y > 0){
			// Jump
			idx = 24 + player.facingRight;
		} else {
			// Fall
			idx = 26 + player.facingRight;
		}
	}
	
	player_sprite(64 + ix, 224 - iy, idx);
	
	// Cursor position.
	ix = (ix + (player.facingRight ? 16 : -16)) & 0xF0;
	iy = (iy + 8) & 0xF0;
	idx = grid_block_at(ix, iy);
	block = GRID[idx];
	if(block > 0 && block != 0xFF){
		cursor_sprite(64 + ix, 208 - iy);
		
		if(JOY_BTN_2(joy)){
			grid_set_block(idx, 0);
		}
	}
}
