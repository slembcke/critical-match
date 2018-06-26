.include "zeropage.inc"

.macpack generic

.importzp px_ticks
.importzp px_sprite_cursor

.import incsp1
.import incsp2
.import OAM

.import METATILE0, METATILE1, METATILE2, METATILE3, METATILE4

CHR0 = $A0
OBJ_COUNT = 6
FRAME_COUNT = 14
ROW_STRIDE = FRAME_COUNT*2

.macro player_right_metasprite n
	.scope
		chr = $A0 + 2*n
		.byte chr+0 + 0*ROW_STRIDE
		.byte chr+1 + 0*ROW_STRIDE
		.byte chr+0 + 1*ROW_STRIDE
		.byte chr+1 + 1*ROW_STRIDE
		.byte chr+0 + 2*ROW_STRIDE
		.byte chr+1 + 2*ROW_STRIDE
	.endscope
.endmacro

.macro player_left_metasprite n
	.scope
		chr = $A0 + 2*n
		.byte chr+1 + 0*ROW_STRIDE
		.byte chr+0 + 0*ROW_STRIDE
		.byte chr+1 + 1*ROW_STRIDE
		.byte chr+0 + 1*ROW_STRIDE
		.byte chr+1 + 2*ROW_STRIDE
		.byte chr+0 + 2*ROW_STRIDE
	.endscope
.endmacro

.rodata

PLAYER_FRAME_DATA:
	.repeat FRAME_COUNT, i
		player_left_metasprite i
		player_right_metasprite i
	.endrepeat

PLAYER_FRAME_ADDRS:
	.repeat 2*FRAME_COUNT, i
		.addr PLAYER_FRAME_DATA + i*OBJ_COUNT
	.endrepeat

.zeropage

sprite_x = tmp1
sprite_y = tmp2
sprite_pal = tmp3

.code

.export _player_sprite
.proc _player_sprite ; u8 x, u8 y, u8 frame
	ldx px_sprite_cursor
	tay ; Save frame number.
	
	; Lowest bit of frame number is left/right.
	eor #$FF
	ror a
	ror a
	ror a
	and #$40
	ora #2 ; Set the palette bits.
	
	; Set attributes.
	sta OAM+ 0+2, x
	sta OAM+ 4+2, x
	sta OAM+ 8+2, x
	sta OAM+12+2, x
	sta OAM+16+2, x
	sta OAM+20+2, x
	
	; Fetch frame address.
	tya ; Restore frame number.
	asl
	tay
	lda PLAYER_FRAME_ADDRS+0, y
	sta ptr1+0
	lda PLAYER_FRAME_ADDRS+1, y
	sta ptr1+1
	
	; Set chr.
	ldy #0
	lda (ptr1), y
	sta OAM+ 0+1, x
	iny
	lda (ptr1), y
	sta OAM+ 4+1, x
	iny
	lda (ptr1), y
	sta OAM+ 8+1, x
	iny
	lda (ptr1), y
	sta OAM+12+1, x
	iny
	lda (ptr1), y
	sta OAM+16+1, x
	iny
	lda (ptr1), y
	sta OAM+20+1, x
	
	; Set x-values.
	ldy #1
	lda (sp), y
	sta OAM+ 4+3, x
	sta OAM+12+3, x
	sta OAM+20+3, x
	sub #8
	sta OAM+ 0+3, x
	sta OAM+ 8+3, x
	sta OAM+16+3, x
	add #8
	
	; Set y-values.
	ldy #0
	lda (sp), y
	sub #9
	sta OAM+16+0, x
	sta OAM+20+0, x
	sub #8
	sta OAM+ 8+0, x
	sta OAM+12+0, x
	sub #8
	sta OAM+ 0+0, x
	sta OAM+ 4+0, x
	
	; Increment sprite cursor.
	ldx px_sprite_cursor
	txa
	add #(24)
	sta px_sprite_cursor
	
	jmp incsp2
.endproc

.export _cursor_sprite
.proc _cursor_sprite ; u8 x, u8 y, u8 height

.rodata
@PALETTES:
	.byte $00, $03, $01, $03

.code
	ldx px_sprite_cursor
	ldx #0
	
	; Set x-values.
	ldy #1
	lda (sp), y
	sub #3
	sta OAM+ 0+3, x
	sta OAM+ 8+3, x
	add #14
	sta OAM+ 4+3, x
	sta OAM+12+3, x
	
	; Set y-values.
	ldy #0
	lda (sp), y
	sub #4
	sta OAM+ 0+0, x
	sta OAM+ 4+0, x
	add #14
	sta OAM+ 8+0, x
	sta OAM+12+0, x
	
	; Set chr.
	lda #$14
	sta OAM+ 0+1, x
	sta OAM+ 4+1, x
	sta OAM+ 8+1, x
	sta OAM+12+1, x
	
	; Calculate palette.
	lda px_ticks
	lsr a
	and #$03
	tay
	lda @PALETTES, y
	tay
	
	; Set attr.
	sta OAM+ 0+2, x
	tya
	ora #$40
	sta OAM+ 4+2, x
	tya
	ora #$80
	sta OAM+ 8+2, x
	tya
	ora #$C0
	sta OAM+12+2, x
	
	; Increment sprite cursor.
	txa
	add #16
	sta px_sprite_cursor
	
	jmp incsp2
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
	add #8
	sta OAM+7, x
	sta OAM+15, x
	
	; Store y-values.
	ldy #0
	lda (sp), y
	sta OAM+0, x
	sta OAM+4, x
	add #8
	sta OAM+8, x
	sta OAM+12, x
	
	; Increment sprite cursor.
	txa
	add #16
	sta px_sprite_cursor
	
	jmp incsp2
.endproc
