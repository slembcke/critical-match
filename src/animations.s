.include "zeropage.inc"

.macpack generic

.import incsp1
.import OAM

BASE_CHR = $A0
FRAME_COUNT = 14
STRIDE = FRAME_COUNT*2
OBJ_COUNT = 6

PAL = 2

.macro right_frame n
	.scope
		attr = $00
		chr = BASE_CHR + 2*n
		.byte  0, chr + 0 + 0*STRIDE, attr,  0
		.byte  0, chr + 1 + 0*STRIDE, attr,  8
		.byte  8, chr + 0 + 1*STRIDE, attr,  0
		.byte  8, chr + 1 + 1*STRIDE, attr,  8
		.byte 16, chr + 0 + 2*STRIDE, attr,  0
		.byte 16, chr + 1 + 2*STRIDE, attr,  8
	.endscope
.endmacro

.macro left_frame n
	.scope
		attr = $70
		chr = BASE_CHR + 2*n
		.byte  0, chr + 1 + 0*STRIDE, attr,  0
		.byte  0, chr + 0 + 0*STRIDE, attr,  8
		.byte  8, chr + 1 + 1*STRIDE, attr,  0
		.byte  8, chr + 0 + 1*STRIDE, attr,  8
		.byte 16, chr + 1 + 2*STRIDE, attr,  0
		.byte 16, chr + 0 + 2*STRIDE, attr,  8
	.endscope
.endmacro

.rodata

.export _animation
_animation:
	.repeat FRAME_COUNT, i
		right_frame i
	.endrepeat

.code

.export _debug_sprite
.proc _debug_sprite
	.zeropage
	sprite_x: .byte 0
	sprite_y: .byte 0
	sprite_pal: .byte 0
	
	.code
	lda #24
	sta sprite_x
	lda #64
	sta sprite_y
	lda #2
	sta sprite_pal
	
	lda #<_animation
	sta ptr1+0
	lda #>_animation
	sta ptr1+1
	
	ldx #0
	ldy #(0*STRIDE)
	:
		lda _animation+0, y
		clc
		adc sprite_y
		sta OAM+0, y
		lda _animation+1, y
		sta OAM+1, y
		lda _animation+2, y
		ora sprite_pal
		sta OAM+2, y
		lda _animation+3, y
		clc
		adc sprite_x
		sta OAM+3, y
		
		iny
		iny
		iny
		iny
		
		inx
		cpx #OBJ_COUNT
		blt :-
	
	rts
.endproc
