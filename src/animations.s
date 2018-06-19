.include "zeropage.inc"

.macpack generic

.import incsp2
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
		.byte 248, 231, chr+0 + 0*STRIDE, attr
		.byte   0, 231, chr+1 + 0*STRIDE, attr
		.byte 248, 239, chr+0 + 1*STRIDE, attr
		.byte   0, 239, chr+1 + 1*STRIDE, attr
		.byte 248, 247, chr+0 + 2*STRIDE, attr
		.byte   0, 247, chr+1 + 2*STRIDE, attr
	.endscope
.endmacro

.rodata

oam_data_r:
	.repeat FRAME_COUNT, i
		right_frame i
	.endrepeat

frame_addrs_r:
	.repeat FRAME_COUNT, i
		.addr oam_data_r + 24*i
	.endrepeat

.zeropage

sprite_x: .byte 0
sprite_y: .byte 0
sprite_pal: .byte 0

.code

.export _debug_sprite
.proc _debug_sprite ; u8 x, u8 y, u8 frame
	; Load metsprite address.
	asl
	tax
	lda frame_addrs_r+0, x
	sta ptr1+0
	lda frame_addrs_r+1, x
	sta ptr1+1
	
	; TODO temp
	ldy #1
	lda (sp), y
	sta sprite_x
	dey
	lda (sp), y
	sta sprite_y
	lda #2
	sta sprite_pal
	
	; Hardcode or no?
	lda #6
	sta sreg
	
	lda #0
	tax
	tay
	:
		; x-pos
		lda (ptr1), y
		iny
		clc
		adc sprite_x
		sta OAM+3, x
		
		; y-pos
		lda (ptr1), y
		iny
		clc
		adc sprite_y
		sta OAM+0, x
		
		; chr
		lda (ptr1), y
		iny
		sta OAM+1, x
		
		; attr
		lda (ptr1), y
		iny
		ora sprite_pal
		sta OAM+2, x
		
		inx
		inx
		inx
		inx
		
		dec sreg
		bne :-
	
	jmp incsp2
.endproc
