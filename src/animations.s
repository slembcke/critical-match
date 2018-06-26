.include "zeropage.inc"

.macpack generic

.importzp px_ticks
.importzp px_sprite_cursor

.import incsp1
.import incsp2
.import OAM

.import METATILE0, METATILE1, METATILE2, METATILE3, METATILE4

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
	lda #2 ; TODO
	sta sprite_pal
	
	; TODO: Unroll.
	lda #6
	sta sreg
	
	ldx px_sprite_cursor
	ldy #0
	:	; x-pos
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

cursor_metasprite:
	.byte 253, 252, $14, $00
	.byte  11, 252, $14, $40
	.byte 253,  10, $14, $80
	.byte  11,  10, $14, $C0

.export _cursor_sprite
.proc _cursor_sprite ; u8 x, u8 y
	; Set x/y offsets.
	sta sprite_y
	ldy #0
	lda (sp), y
	sta sprite_x
	
	; TODO: Unroll.
	lda #4
	sta sreg
	
	ldx px_sprite_cursor
	ldy #0
	:	; x-pos
		lda cursor_metasprite, y
		iny
		clc
		adc sprite_x
		sta OAM+3, x
		
		; y-pos
		lda cursor_metasprite, y
		iny
		clc
		adc sprite_y
		sta OAM+0, x
		
		; chr
		lda cursor_metasprite, y
		iny
		sta OAM+1, x
		
		; attr
		lda px_ticks
		lsr
		and #$03
		ora cursor_metasprite, y
		iny
		sta OAM+2, x
		
		inx
		inx
		inx
		inx
		
		dec sreg
		bne :-
	
	stx px_sprite_cursor
	jmp incsp1
.endproc

.export _block_sprite
.proc _block_sprite ; u8 x, u8 y, u8 frame
	ldx px_sprite_cursor
	
	; Store metatile index in y
	tay
	
	; Store chr.
	lda METATILE0, y
	sta OAM+1, x
	lda METATILE1, y
	sta OAM+5, x
	lda METATILE2, y
	sta OAM+9, x
	lda METATILE3, y
	sta OAM+13, x
	
	; Store attr.
	lda METATILE4, y
	and #$03
	sta OAM+2, x
	sta OAM+6, x
	sta OAM+10, x
	sta OAM+14, x
	
	; Store x-values.
	ldy #1
	lda (sp), y
	sta OAM+3, x
	sta OAM+11, x
	clc
	adc #8
	sta OAM+7, x
	sta OAM+15, x
	
	; Store y-values.
	ldy #0
	lda (sp), y
	sta OAM+0, x
	sta OAM+4, x
	clc
	adc #8
	sta OAM+8, x
	sta OAM+12, x
	
	; Increment sprite cursor.
	txa
	clc
	adc #16
	sta px_sprite_cursor
	
	jmp incsp2
.endproc
