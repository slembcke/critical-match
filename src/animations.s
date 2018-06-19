.include "zeropage.inc"

.macpack generic

.import incsp2
.import OAM

FRAME_COUNT = 14
STRIDE = FRAME_COUNT*2
OBJ_COUNT = 6

.macro player_metasprite n
	.scope
		attr = $00
		chr = $A0 + 2*n
		.byte 248, 231, chr+0 + 0*STRIDE, attr
		.byte   0, 231, chr+1 + 0*STRIDE, attr
		.byte 248, 239, chr+0 + 1*STRIDE, attr
		.byte   0, 239, chr+1 + 1*STRIDE, attr
		.byte 248, 247, chr+0 + 2*STRIDE, attr
		.byte   0, 247, chr+1 + 2*STRIDE, attr
	.endscope
.endmacro

.rodata

player_frame00: player_metasprite 0
player_frame01: player_metasprite 1
player_frame02: player_metasprite 2
player_frame03: player_metasprite 3
player_frame04: player_metasprite 4
player_frame05: player_metasprite 5
player_frame06: player_metasprite 6
player_frame07: player_metasprite 7
player_frame08: player_metasprite 8
player_frame09: player_metasprite 9
player_frame10: player_metasprite 10
player_frame11: player_metasprite 11
player_frame12: player_metasprite 12
player_frame13: player_metasprite 13

frame_addrs_r:
	.addr player_frame00
	.addr player_frame01
	.addr player_frame02
	.addr player_frame03
	.addr player_frame04
	.addr player_frame05
	.addr player_frame06
	.addr player_frame07
	.addr player_frame08
	.addr player_frame09
	.addr player_frame10
	.addr player_frame11
	.addr player_frame12
	.addr player_frame13

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
