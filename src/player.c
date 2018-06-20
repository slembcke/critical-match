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
	
	// TODO Collision detection
	if(player.pos_y < 16*0x100){
		player.pos_y = 16*0x100;
		player.vel_y = MAX(0, player.vel_y);
		
		player.grounded = true;
		if(!JOY_BTN_1(joy)) player.jump_ticks = PLAYER_JUMP_TICKS;
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
			idx = 24 + player.facingRight;
		}
	}
	
	player_sprite(64 + ix, 224 - iy, idx);
}