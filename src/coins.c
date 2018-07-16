#include "pixler/pixler.h"
#include "shared.h"

#define COIN_COUNT 16
u8 COIN_BLOCK_IDX[COIN_COUNT];
u8 COIN_TIMEOUT[COIN_COUNT];

#define FOO(t) (-t*7/2 + t*t*2/5)
static const s8 COIN_ANIM_Y[] = {
	FOO(0x0), FOO(0x1), FOO(0x2), FOO(0x3),
	FOO(0x4), FOO(0x5), FOO(0x6), FOO(0x7),
	FOO(0x8), FOO(0x9), FOO(0xA), FOO(0xB),
	FOO(0xC), FOO(0xD), FOO(0xE), FOO(0xF),
};

void coins_init(void){
	for(idx = 0; idx < COIN_COUNT; ++idx){
		COIN_TIMEOUT[idx] = sizeof(COIN_ANIM_Y);
	}
}

void coins_draw(void){
	register u8 i, t;
	
	for(i = 0; i < COIN_COUNT; ++i){
		t = COIN_TIMEOUT[i];
		
		if(t < COIN_COUNT){
			ix = (t ^ 0xFF);
			iy = COIN_ANIM_Y[t];
			
			idx = COIN_BLOCK_IDX[i];
			ix += ( 68 + (u8)((idx & 0x07) << 4));
			iy += (210 - (u8)((idx & 0xF8) << 1));
			px_spr(ix, iy, 0x00, 0x95);
			
			++COIN_TIMEOUT[i];
		}
	}
}

static u8 coin_cursor = 0;
void coin_at(u8 idx){
	COIN_BLOCK_IDX[coin_cursor] = idx;
	COIN_TIMEOUT[coin_cursor] = 0;
	
	coin_cursor = (coin_cursor + 1) & (COIN_COUNT - 1);
}
