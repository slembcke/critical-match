#include "pixler/pixler.h"
#include "shared.h"

#define COIN_COUNT 16
u8 COIN_BLOCK_IDX[COIN_COUNT];
u8 COIN_TIMEOUT[COIN_COUNT];

#define ANIM(t) (-t*7/2 + t*t*2/5)
static const s8 COIN_ANIM_Y[] = {
	ANIM(0x0), ANIM(0x1), ANIM(0x2), ANIM(0x3),
	ANIM(0x4), ANIM(0x5), ANIM(0x6), ANIM(0x7),
	ANIM(0x8), ANIM(0x9), ANIM(0xA), ANIM(0xB),
	ANIM(0xC), ANIM(0xD), ANIM(0xE), ANIM(0xF),
};

static const u8 COIN_ATTRS[] = {2, 1, 0, 2, 0, 1, 0, 2, 0, 3, 2, 3, 1, 3, 1, 3};

#define ATTR_FLIP(x) (x & 0x1)
#define ATTR_SHIFT(x) (x & 0x2)

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
			ix = t;
			if(ATTR_SHIFT(COIN_ATTRS[i])) ix >>= 1;
			if(ATTR_FLIP(COIN_ATTRS[i])) ix ^= 0xFF;
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
void coins_add_at(u8 idx){
	(COIN_BLOCK_IDX + 0)[coin_cursor] = idx;
	(COIN_BLOCK_IDX + 1)[coin_cursor] = idx;
	(COIN_TIMEOUT + 0)[coin_cursor] = 0;
	(COIN_TIMEOUT + 1)[coin_cursor] = 0;
	
	coin_cursor = (coin_cursor + 2) & (COIN_COUNT - 1);
}
