#include <string.h>

#include <nes.h>
#include <joystick.h>

#include "pixler/pixler.h"
#include "shared.h"

// 0x100, 0x180, 0x1C0
#define PLAYER_MAX_SPEED 0x01C0
#define PLAYER_ACCEL 0x0060
#define PLAYER_GRAVITY 0x0080
#define PLAYER_MAX_FALL (6*PLAYER_MAX_SPEED/2)
#define PLAYER_JUMP 0x0380
// 5, 8
#define PLAYER_JUMP_TICKS 10
#define MAX_Y ((16 << 8)*(GRID_H - 2))

#define ENABLE_DOUBLE_JUMP 0
#define ENABLE_GRAPPLE 1

typedef struct {
	u16 pos_x, pos_y;
	s16 vel_x, vel_y;
	
	// Desired movement.
	s16 move;
	
	bool facingRight;
	bool grounded;
	
	// Is double jump ready?
	bool double_jump;
	// Remaining ticks of jump power.
	u8 jump_ticks;
	
	// Selected block under the cursor_y.
	u8 cursor_idx;
	
	u8 block_x, block_y;
	u8 grapple_y;
	u8 blocks_held[GRID_H];
	
	// Input values
	u8 joy, prev_joy;
} Player;

static Player player;

// Used by the tutorial.
u16 *player_x = &player.pos_x, *player_y = &player.pos_y;

void player_init(void){
	bzero(&player, sizeof(player));
	player.pos_x = 64 << 8;
	player.pos_y = 16 << 8;
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
			if(player.jump_ticks == PLAYER_JUMP_TICKS) sound_play(SOUND_JUMP);
			
			player.vel_y = PLAYER_JUMP;
			--player.jump_ticks;
			
		}
		
		if(ENABLE_DOUBLE_JUMP && !JOY_BTN_1(player.prev_joy)){
			// If you are grounded when you first jump, enable double jump.
			player.double_jump = player.grounded;
		}
	} else if(player.grounded || player.double_jump){
		player.jump_ticks = PLAYER_JUMP_TICKS;
	} else {
		// Deplete the jump ticks if in the air and not jumping.
		player.jump_ticks = 0;
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
	iy = (player.pos_y >> 8) + 4;
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
		if(ENABLE_GRAPPLE && JOY_UP(player.joy)){
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
		// We are holding a block, and need to make sure enough space is clear to drop it.
		// Abusing ix to copy idx.
		for(iy = 0, ix = idx; player.blocks_held[iy]; ++iy, ix += GRID_W){
			if(
				// Don't allow blocks to be placed above the top row of the board.
				ix > GRID_BYTES - GRID_W ||
				// Don't allow blocks to be placed where other blocks are falling.
				GRID[ix] != BLOCK_EMPTY
			) return;
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

void player_draw(void){
	ix = player.pos_x >> 8;
	iy = player.pos_y >> 8;
	
	// Select frame index.
	if(player.grounded){
		if((player.vel_x >> 8) == 0){
			// Idle
			idx = ((px_ticks >> 2) & 0x6) + 16 + player.facingRight;
		} else {
			// Run
			idx = ((ix >> 1) & 14) + player.facingRight;
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

void player_draw_blocks(void){
	// Draw blocks.
	if(player.grapple_y){
		ix = player.block_x - 5;
		iy = player.block_y;
	} else {
		ix = ( 64 -  8) + (player.pos_x >> 8);
		iy = (224 - 32) - (player.pos_y >> 8);
	}
	
	for(idx = 0; player.blocks_held[idx]; ++idx){
		block_sprite(ix, iy, player.blocks_held[idx] & BLOCK_GFX_MASK);
		iy -= 16;
	}
}

void player_draw_grapple(void){
	u8 player_x, player_y;
	register u8 dx, dy, eps = 0, x_inc;
	
	if(player.grapple_y == 0) return;
	player_x = 60 + (player.pos_x >> 8);
	player_y = 208 - ((player.pos_y >> 8) & ~0x7);
	
	// px_spr(player.block_x, player.block_y, 0x00, 'O');
	// px_spr(player_x, player_y, 0x00, 'O');
	
	if(player_y - 32 <= player.block_y){
		// Blocks are already right above the player.
		player.grapple_y = 0;
		grid_pause_semaphore(-1);
		return;
	}
	
	dy = (u8)(player_y - player.block_y)>>4;
	if(player.block_x <= player_x){
		dx = (u8)(player_x - player.block_x)>>1;
		x_inc = -1;
	} else {
		dx = (u8)(player.block_x - player_x)>>1;
		x_inc = 1;
	}
	
	ix = player_x;
	iy = player_y;
	while(true){
		iy -= 8;
		eps += dx;
		while(eps >= dy){
			ix += x_inc;
			eps -= dy;
		}
		
		if(iy <= player.grapple_y) break;
		
		px_spr(ix, iy, 0x01, 0x08);
	}
	// Draw the hook.
	px_spr(ix, iy, 0x03, 0x07);
	
	// Pull the block towards the player.
	// TODO Not drawing code. Care?
	if(player.grapple_y >= player.block_y){
			player.grapple_y -= 16;
	} else {
		player.block_y += 16;
		if(player_y - 32 <= player.block_y){
			// Grapple animation has finished.
			player.grapple_y = 0;
			grid_pause_semaphore(-1);
		} else {
			player.grapple_y += 16;
		}
		
		eps += dx/2;
		while(eps >= dy){
			player.block_x -= x_inc;
			eps -= dy;
		}
	}
}

void player_pick_up(void){
	// Flush any pending PPU writes.
	px_wait_nmi();
	
	if(JOY_UP(player.joy)){
		player.grapple_y = 240;
		player.block_x = grid_block_x(player.cursor_idx, 5);
		player.block_y = grid_block_y(player.cursor_idx, 0);
		grid_pause_semaphore(1);
	} else {
		player.grapple_y = 0;
	}
	
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
	
	// Flush the PPU buffer again.
	px_wait_nmi();
	sound_play(SOUND_PICKUP);
}

void player_drop(void){
	if(player.cursor_idx && player.grapple_y == 0){
		// Flush any pending PPU writes.
		px_wait_nmi();
		
		for(idx = player.cursor_idx, iy = 0; player.blocks_held[iy]; idx += GRID_W, iy += 1){
			grid_set_block(idx, player.blocks_held[iy]);
			player.blocks_held[iy] = 0;
			
			if(JOY_DOWN(player.joy)) player.pos_y += 16*256;
		}
		
		player.cursor_idx = 0;
		
		grid_update_column_height();
		
		// Flush the PPU buffer again.
		px_wait_nmi();
		sound_play(SOUND_DROP);
	} else {
		// TODO play "denied" sound?
	}
}

void player_update(u8 joy){
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
		ix = grid_block_x(idx,  0);
		iy = grid_block_y(idx, -5);
		
		// TODO rewrite in inline asm.
		// Calculate cursor height into idx.
		if(player.blocks_held[0] == BLOCK_EMPTY){
			// Calculate cursor height from blocks on grid.
			for(idx = player.cursor_idx; GRID[idx] != BLOCK_EMPTY && idx < GRID_BYTES; idx += GRID_W);
			idx = (idx - player.cursor_idx)/8;
		} else {
			// Calculate cursor height from held blocks.
			for(idx = 1; player.blocks_held[idx] != BLOCK_EMPTY && idx < GRID_H; ++idx);
		}
		
		cursor_sprite(ix, iy, idx);
	}
	
	player.prev_joy = player.joy;
}
