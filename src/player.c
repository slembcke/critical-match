#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"

#define PLAYER_MAX_SPEED 0x0180
#define PLAYER_ACCEL 0x0060
#define PLAYER_GRAVITY 0x00A0
#define PLAYER_MAX_FALL (6*PLAYER_MAX_SPEED/2)
#define PLAYER_JUMP 0x03C0
#define PLAYER_JUMP_TICKS 5
#define MAX_Y ((16 << 8)*(GRID_H - 2))

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
	
	u8 blocks_held[GRID_H];
	
	// Input values
	u8 joy, prev_joy;
} Player;

static const Player INIT = {64 << 8, 16 << 8, 0, 0};
static Player player;

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
	ix = (player.pos_x >> 8)/16;
	iy = 16*COLUMN_HEIGHT[ix] + 16;
	if(player.pos_y < 256*iy) player.pos_y = 256*iy;
	if(player.pos_y > MAX_Y) player.pos_y = MAX_Y;
	
	ix = (player.pos_x >> 8);
	iy = (player.pos_y >> 8) - 1;
	
	// Ground detection.
	player.grounded = false;
	
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
			// Start at the left/right block.
			idx += (player.facingRight ? 1 : -1);
			
			if(GRID[idx] == BLOCK_EMPTY){
				// If that block is empty, try the blok below it too.
				if((GRID - GRID_W)[idx] == BLOCK_EMPTY) idx -= GRID_W;
			} else {
				// Try moving up a block to loop for a clear spot.
				idx += GRID_W;
			}
		}
	} else {
		if(JOY_UP(player.joy)){
			// Search upwards for a block to grapple.
			while(GRID[idx] == BLOCK_EMPTY){
				idx += GRID_W;
				
				// Check if we've passed the top of the grid.
				if(idx >= GRID_BYTES) return;
			}
		} else if(JOY_DOWN(player.joy)){
			idx -= GRID_W;
		} else {
			// Get the block in front of the player.
			if(player.facingRight){
				idx += 1;
			} else {
				idx -= 1;
			}
			
			// Move down a row if it's empty.
			if(GRID[idx] == BLOCK_EMPTY) idx -= GRID_W;
		}
	}
	
	if(player.blocks_held[0] != BLOCK_EMPTY){
		// We are holding a block.
		// Need to make sure enough space is clear to drop it.
		for(iy = 0, ix = idx; player.blocks_held[iy]; ++iy, ix += GRID_W){
			if(GRID[ix] != BLOCK_EMPTY) return;
		}
		
		player.cursor_idx = idx;
	} else if(GRID[idx] > BLOCK_BORDER){
		// Not holding a block, and there is one to pick up.
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
	// TODO Update column height.
	for(iy = 0, idx = player.cursor_idx; idx < GRID_BYTES; ++iy, idx += GRID_W){
		if(
			// Stop for empty spaces...
			GRID[idx] == BLOCK_EMPTY ||
			// ... or a block that is already matching.
			(GRID[idx] & BLOCK_STATUS_UNLOCKED)
		) break;
		
		player.blocks_held[iy] = GRID[idx];
		if(idx < GRID_BYTES - 8){
			grid_set_block(idx, BLOCK_EMPTY);
		} else {
			// The top row of the grid is not shown.
			// Don't change the tile, but do set the value.
			GRID[idx] = BLOCK_EMPTY;
		}
	}
	
	player.cursor_idx = 0;
	
	grid_update_column_height();
	
	// Wait for the PPU buffer to flush.
	px_wait_nmi();
}

void player_drop(void){
	if(player.cursor_idx){
		for(idx = player.cursor_idx, iy = 0; player.blocks_held[iy]; idx += GRID_W, iy += 1){
			grid_set_block(idx, player.blocks_held[iy]);
			player.blocks_held[iy] = 0;
			
			if(JOY_DOWN(player.joy)) player.pos_y += 16*256;
		}
		
		player.cursor_idx = 0;
		
		grid_update_column_height();
		
		// Wait for the PPU buffer to flush.
		px_wait_nmi();
	} else {
		// TODO play "denied" sound?
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
	if(JOY_BTN_2(player.joy) && !JOY_BTN_2(player.prev_joy) && player.cursor_idx){
		if(player.blocks_held[0]){
			player_drop();
		} else {
			player_pick_up();
		}
	}
	
	// Draw cursor.
	if(player.cursor_idx){
		// Convert the block index to the base sprite coord.
		// TODO Should cursor_sprite do this? Meh...
		ix =  (64 + (u8)((idx & 0x07) << 4));
		iy = -(48 + (u8)((idx & 0xF8) << 1));
		
		// Calculate cursor height into idx.
		if(player.blocks_held[0]){
			for(idx = 1; player.blocks_held[idx] != BLOCK_EMPTY && idx < GRID_H; ++idx);
		} else {
			for(idx = player.cursor_idx; GRID[idx] != BLOCK_EMPTY && idx < GRID_BYTES; idx += GRID_W);
			idx = (idx - player.cursor_idx)/8;
		}
		
		cursor_sprite(ix, iy, idx);
	}
	
	// Draw blocks.
	ix = ( 64 -  8) + (player.pos_x >> 8);
	iy = (224 - 32) - (player.pos_y >> 8);
	for(idx = 0; player.blocks_held[idx]; ++idx){
		block_sprite(ix, iy, player.blocks_held[idx] & ~BLOCK_STATUS_MASK);
		iy -= 16;
	}
	
	player_sprite_draw();
	
	player.prev_joy = player.joy;
}
