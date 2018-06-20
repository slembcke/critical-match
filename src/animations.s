.include "zeropage.inc"

.macpack generic

.importzp px_sprite_cursor

.import incsp2
.import OAM

OBJ_COUNT = 6
FRAME_COUNT = 14
FRAME_SIZE = 4*OBJ_COUNT
ROW_STRIDE = FRAME_COUNT*2

.macro player_right_metasprite n
	.scope
		attr = $00
		chr = $A0 + 2*n
		.byte 248, 231, chr+0 + 0*ROW_STRIDE, attr
		.byte   0, 231, chr+1 + 0*ROW_STRIDE, attr
		.byte 248, 239, chr+0 + 1*ROW_STRIDE, attr
		.byte   0, 239, chr+1 + 1*ROW_STRIDE, attr
		.byte 248, 247, chr+0 + 2*ROW_STRIDE, attr
		.byte   0, 247, chr+1 + 2*ROW_STRIDE, attr
	.endscope
.endmacro

.macro player_left_metasprite n
	.scope
		attr = $40
		chr = $A0 + 2*n
		.byte 248, 231, chr+1 + 0*ROW_STRIDE, attr
		.byte   0, 231, chr+0 + 0*ROW_STRIDE, attr
		.byte 248, 239, chr+1 + 1*ROW_STRIDE, attr
		.byte   0, 239, chr+0 + 1*ROW_STRIDE, attr
		.byte 248, 247, chr+1 + 2*ROW_STRIDE, attr
		.byte   0, 247, chr+0 + 2*ROW_STRIDE, attr
	.endscope
.endmacro

.rodata

player_frame_data:
	.repeat FRAME_COUNT, i
		player_left_metasprite i
		player_right_metasprite i
	.endrepeat

player_frames:
	.repeat 2*FRAME_COUNT, i
		.addr player_frame_data + i*FRAME_SIZE
	.endrepeat

.zeropage

sprite_x: .byte 0
sprite_y: .byte 0
sprite_pal: .byte 0

.code

.export _player_sprite
.proc _player_sprite ; u8 x, u8 y, u8 frame
	; Load metsprite address.
	asl
	tax
	lda player_frames+0, x
	sta ptr1+0
	lda player_frames+1, x
	sta ptr1+1
	
	; Set x/y offsets.
	ldy #1
	lda (sp), y
	sta sprite_x
	dey
	lda (sp), y
	sta sprite_y
	lda #2
	sta sprite_pal
	
	; TODO: Hardcode or no?
	lda #6
	sta sreg
	
	ldx px_sprite_cursor
	ldy #0
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
	
	stx px_sprite_cursor
	jmp incsp2
.endproc

.export _px_spr_end
.proc _px_spr_end
	; TODO Infinite loop if cursor is not aligned!
	lda #240 ; y positions past 240 are offscreen.
	; Move sprites offscreen.
	ldx px_sprite_cursor
	:	sta OAM, x ; Store y position;
		; Skip 4 bytes to the next sprite.
		inx
		inx
		inx
		inx
		bne :-
	
	stx px_sprite_cursor
	rts
.endproc
