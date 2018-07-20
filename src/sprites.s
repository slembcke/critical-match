.include "zeropage.inc"
.include "pixler/pixler.inc"

.macpack generic

.importzp px_ticks
.importzp px_sprite_cursor

.import incsp1
.import incsp2

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
	sta OAM_ATTR+ 0, x
	sta OAM_ATTR+ 4, x
	sta OAM_ATTR+ 8, x
	sta OAM_ATTR+12, x
	sta OAM_ATTR+16, x
	sta OAM_ATTR+20, x
	
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
	sta OAM_CHR+ 0, x
	iny
	lda (ptr1), y
	sta OAM_CHR+ 4, x
	iny
	lda (ptr1), y
	sta OAM_CHR+ 8, x
	iny
	lda (ptr1), y
	sta OAM_CHR+12, x
	iny
	lda (ptr1), y
	sta OAM_CHR+16, x
	iny
	lda (ptr1), y
	sta OAM_CHR+20, x
	
	; Set x-values.
	ldy #1
	lda (sp), y
	sta OAM_X+ 4, x
	sta OAM_X+12, x
	sta OAM_X+20, x
	sub #8
	sta OAM_X+ 0, x
	sta OAM_X+ 8, x
	sta OAM_X+16, x
	add #8
	
	; Set y-values.
	ldy #0
	lda (sp), y
	sub #9
	sta OAM_Y+16, x
	sta OAM_Y+20, x
	sub #8
	sta OAM_Y+ 8, x
	sta OAM_Y+12, x
	sub #8
	sta OAM_Y+ 0, x
	sta OAM_Y+ 4, x
	
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
	@PALETTE_ANIM: .byte $00, $03, $01, $03

.code
	ldx px_sprite_cursor
	asl a
	asl a
	asl a
	asl a
	sub #6
	sta tmp1
	
	; Set y-values.
	ldy #0
	lda (sp), y
	add #8
	sta OAM_Y+ 8, x
	sta OAM_Y+12, x
	sub tmp1
	sta OAM_Y+ 0, x
	sta OAM_Y+ 4, x
	
	; Set x-values.
	ldy #1
	lda (sp), y
	sub #1
	sta OAM_X+ 0, x
	sta OAM_X+ 8, x
	add #10
	sta OAM_X+ 4, x
	sta OAM_X+12, x
	
	; Set chr.
	lda #$04
	sta OAM_CHR+ 0, x
	sta OAM_CHR+ 4, x
	sta OAM_CHR+ 8, x
	sta OAM_CHR+12, x
	
	; Calculate palette.
	lda px_ticks
	lsr a
	and #$03
	tay
	lda @PALETTE_ANIM, y
	tay
	
	; Set attr.
	sta OAM_ATTR+ 0, x
	tya
	ora #$40
	sta OAM_ATTR+ 4, x
	tya
	ora #$80
	sta OAM_ATTR+ 8, x
	tya
	ora #$C0
	sta OAM_ATTR+12, x
	
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
	sta OAM_CHR+ 0, x
	lda METATILE1, y
	sta OAM_CHR+ 4, x
	lda METATILE2, y
	sta OAM_CHR+ 8, x
	lda METATILE3, y
	sta OAM_CHR+12, x
	
	; Store attr.
	lda METATILE4, y
	and #$03
	sta OAM_ATTR+ 0, x
	sta OAM_ATTR+ 4, x
	sta OAM_ATTR+ 8, x
	sta OAM_ATTR+12, x
	
	; Store x-values.
	ldy #1
	lda (sp), y
	sta OAM_X+ 0, x
	sta OAM_X+ 8, x
	add #8
	sta OAM_X+ 4, x
	sta OAM_X+12, x
	
	; Store y-values.
	ldy #0
	lda (sp), y
	sta OAM_Y+ 0, x
	sta OAM_Y+ 4, x
	add #8
	sta OAM_Y+ 8, x
	sta OAM_Y+12, x
	
	; Increment sprite cursor.
	txa
	add #16
	sta px_sprite_cursor
	
	jmp incsp2
.endproc

.export _explosion_sprite
.proc _explosion_sprite ; u8 x, u8 y, u8 frame
	ldx px_sprite_cursor
	
	; Store chr.
	asl
	add #$20
	sta OAM_CHR+ 0, x
	adc #1
	sta OAM_CHR+ 4, x
	adc #15
	sta OAM_CHR+ 8, x
	adc #1
	sta OAM_CHR+12, x
	
	; Store attr.
	lda #$01
	sta OAM_ATTR+ 0, x
	sta OAM_ATTR+ 4, x
	sta OAM_ATTR+ 8, x
	sta OAM_ATTR+12, x
	
	; Store x-values.
	ldy #1
	lda (sp), y
	sta OAM_X+ 0, x
	sta OAM_X+ 8, x
	add #8
	sta OAM_X+ 4, x
	sta OAM_X+12, x
	
	; Store y-values.
	ldy #0
	lda (sp), y
	sta OAM_Y+ 0, x
	sta OAM_Y+ 4, x
	add #8
	sta OAM_Y+ 8, x
	sta OAM_Y+12, x
	
	; Increment sprite cursor.
	txa
	add #16
	sta px_sprite_cursor
	
	jmp incsp2
.endproc
