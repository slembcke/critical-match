.include "zeropage.inc"

.macpack generic

.importzp px_sprite_cursor

.import OAM

.export _px_spr_end
.proc _px_spr_end
	; TODO Infinite loop if cursor is not aligned!
	lda #240 ; y positions past 240 are offscreen.
	ldx px_sprite_cursor
	:	sta OAM, x ; Store y position;
		; Skip 4 bytes to the next sprite.
		inx
		inx
		inx
		inx
		bne :-
	
	; x = 0
	stx px_sprite_cursor
	rts
.endproc
