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
void block_sprite(u8 x, u8 y, u8 block);
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
	
	u8 cursor_x, cursor_y;
	// Selected block under the cursor.
	u8 cursor_idx;
	
	u8 blocks_held[10];
	
	// Input values
	u8 joy, prev_joy;
} Player;

static const Player INIT = {64 << 8, 16 << 8, 0, 0};
static Player player = {};

void player_init(void){
	player = INIT;
	player.facingRight = true;
}

#define grid_block_at(x, y) (((y >> 1) & 0xF8) | ((x >> 4)))
static u16 bx, by, block;

static void player_update_motion(void){
	player.move = 0;
	if(JOY_LEFT(player.joy)) player.move -= PLAYER_MAX_SPEED;
	if(JOY_RIGHT(player.joy)) player.move += PLAYER_MAX_SPEED;
	
	player.pos_x += player.vel_x;
	player.pos_y += player.vel_y;
	
	player.vel_x += CLAMP(player.move - player.vel_x, -PLAYER_ACCEL, PLAYER_ACCEL);
	player.vel_y = MAX(-PLAYER_MAX_FALL, player.vel_y - PLAYER_GRAVITY);
	
	if(JOY_BTN_1(player.joy)){
		if(player.jump_ticks > 0){
			player.vel_y = PLAYER_JUMP;
			--player.jump_ticks;
		}
	} else if(player.grounded){
		player.jump_ticks = PLAYER_JUMP_TICKS;
	}
}

static void player_collide(void){
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
}

static void player_cursor_update(void){
	player.cursor_idx = 0;
	
	if(player.blocks_held[0]){
		// TODO placement cursor
	} else {
		if(JOY_UP(player.joy)){
			// TODO up cursor.
			return;
		} else if(JOY_DOWN(player.joy)){
			ix = 0;
			iy = -8;
		} else {
			// Left or right.
			ix = (player.facingRight ? 16 : -16);
			iy = 8;
		}
		
		ix = (ix + (player.pos_x >> 8)) & 0xF0;
		iy = (iy + (player.pos_y >> 8)) & 0xF0;
		idx = grid_block_at(ix, iy);
		block = GRID[idx];
		if(block > 0 && block != 0xFF){
			player.cursor_x =  64 + ix;
			player.cursor_y = 208 - iy;
			player.cursor_idx = idx;
		}
	}
}

static void player_facing_update(void){
	if(player.vel_x > 0){
		player.facingRight = true;
	} else if(player.vel_x < 0){
		player.facingRight = false;
	} else if(JOY_RIGHT(player.prev_joy)){
		player.facingRight = true;
	} else if(JOY_LEFT(player.prev_joy)){
		player.facingRight = false;
	}
}

static void player_sprite_draw(void){
	ix = player.pos_x >> 8;
	iy = player.pos_y >> 8;
	
	// Select frame index.
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
}

void player_pick_up(void){
	for(idx = player.cursor_idx, iy = 0; GRID[idx]; idx += GRID_W, ++iy){
		player.blocks_held[iy] = GRID[idx];
		grid_set_block(idx, 0);
	}
}

void player_tick(u8 joy){
	player.joy = joy;
	
	// Update player state.
	player_update_motion();
	player_collide();
	player_facing_update();
	
	// Update action.
	player_cursor_update();
	if(!JOY_BTN_2(player.joy) && JOY_BTN_2(player.prev_joy) && player.cursor_idx){
		player_pick_up();
	}
	
	// Draw
	if(player.cursor_idx){
		// TODO cursor height?
		cursor_sprite(player.cursor_x, player.cursor_y);
	}
	
	ix = ( 64 -  8) + (player.pos_x >> 8);
	iy = (224 - 32) - (player.pos_y >> 8);
	for(idx = 0; player.blocks_held[idx]; ++idx){
		block_sprite(ix, iy, player.blocks_held[idx]);
		iy -= 16;
	}
	
	player_sprite_draw();
	
	player.prev_joy = player.joy;
}
