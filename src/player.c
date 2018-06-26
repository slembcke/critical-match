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
void cursor_sprite(u8 x, u8 y, u8 height);

typedef struct {
	u16 pos_x, pos_y;
	s16 vel_x, vel_y;
	
	// Desired movement.
	s16 move;
	
	bool facingRight;
	bool grounded;
	
	// Remaining ticks of jump power.
	u8 jump_ticks;
	
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
static u16 bx, by;
static u8 block;

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
	
	// Calculate the block index under the player's center.
	ix = (player.pos_x >> 8) + 0;
	iy = (player.pos_y >> 8) + 8;
	idx = grid_block_at(ix, iy);
	
	if(player.blocks_held[0]){
		if(JOY_UP(player.joy)){
			return; // Cannot drop blocks upwards.
		} else if(JOY_DOWN(player.joy)){
			// Place blocks in the character's current square, do nothing.
		} else {
			// Start down a row + left/right.
			if(player.facingRight){
				idx += -GRID_W + 1;
			} else {
				idx += -GRID_W - 1;
			}
			
			// Move up twice to look for clear locations.
			if(GRID[idx] != 0) idx += GRID_W;
			if(GRID[idx] != 0) idx += GRID_W;
		}
	} else {
		if(JOY_UP(player.joy)){
			// Search upwards for a block to grapple.
			for(; GRID[idx] == 0; idx += GRID_W){
				// Check if we missed.
				if(idx > GRID_W*GRID_H) return;
			}
		} else if(JOY_DOWN(player.joy)){
			idx -= GRID_W;
		} else if(player.facingRight){
			idx += 1;
		} else {
			idx -= 1;
		}
	}
	
	block = GRID[idx];
	if(!player.blocks_held[0] && block > 0 && block != 0xFF){
		player.cursor_idx = idx;
	} else if(player.blocks_held[0] && block == 0){
		player.cursor_idx = idx;
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
	for(idx = player.cursor_idx, iy = 0; GRID[idx]; idx += GRID_W, iy += 1){
		player.blocks_held[iy] = GRID[idx];
		grid_set_block(idx, 0);
	}
	
	// Wait for the PPU buffer to flush.
	px_wait_nmi();
}

void player_drop(void){
	// TODO needs to check if there is enough space.
	for(idx = player.cursor_idx, iy = 0; player.blocks_held[iy]; idx += GRID_W, iy += 1){
		if(GRID[idx] != 0){
			// Area not clear, can't put down the stack.
			// TODO sound?
			return;
		}
	}
	
	for(idx = player.cursor_idx, iy = 0; player.blocks_held[iy]; idx += GRID_W, iy += 1){
		grid_set_block(idx, player.blocks_held[iy]);
		player.blocks_held[iy] = 0;
		
		if(JOY_DOWN(player.joy)) player.pos_y += 16*256;
	}
	
	// Wait for the PPU buffer to flush.
	px_wait_nmi();
}

void player_tick(u8 joy){
	player.joy = joy;
	
	// Update player state.
	player_update_motion();
	player_collide();
	player_facing_update();
	
	// Update action.
	player_cursor_update();
	if(JOY_BTN_2(player.joy) && !JOY_BTN_2(player.prev_joy) && player.cursor_idx){
		if(player.blocks_held[0]){
			player_drop();
		} else {
			player_pick_up();
		}
	}
	
	// Draw cursor.
	if(player.cursor_idx){
		ix =  (64 + (u8)((idx & 0x07) << 4));
		iy = -(48 + (u8)((idx & 0xF8) << 1));
		cursor_sprite(ix, iy, 1);
	}
	
	// Draw blocks.
	ix = ( 64 -  8) + (player.pos_x >> 8);
	iy = (224 - 32) - (player.pos_y >> 8);
	for(idx = 0; player.blocks_held[idx]; ++idx){
		block_sprite(ix, iy, player.blocks_held[idx]);
		iy -= 16;
	}
	
	player_sprite_draw();
	
	player.prev_joy = player.joy;
}
