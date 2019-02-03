.include "zeropage.inc"
.include "pixler.inc"

.macpack generic

.importzp px_sprite_cursor

.import incsp3


.export _px_spr_clear
.proc _px_spr_clear
	lda #0
	sta px_sprite_cursor
	
	jmp _px_spr_end
.endproc

.export _px_spr
.proc _px_spr ; u8 x, u8, y, u8 attr, u8 chr
	ldx px_sprite_cursor
	
	; Store chr.
	sta OAM_CHR, x
	
	; Store x.
	ldy #2
	lda (sp), y
	sta OAM_X, x
	
	; Store y.
	dey
	lda (sp), y
	sta OAM_Y, x
	
	; Store attr.
	dey
	lda (sp), y
	sta OAM_ATTR, x
	
	txa
	add #4
	sta px_sprite_cursor
	
	jmp incsp3
.endproc

.export _px_spr_end
.proc _px_spr_end
	; TODO Infinite loop if cursor is not aligned!
	lda #240 ; y positions past 240 are offscreen.
	ldx px_sprite_cursor
	:	sta OAM_Y, x
		; Skip to the next sprite.
		inx
		inx
		inx
		inx
		bne :-
	
	; x = 0
	stx px_sprite_cursor
	rts
.endproc
