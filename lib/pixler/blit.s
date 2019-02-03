.include "zeropage.inc"
.include "pixler.inc"

.import popax

.export _px_blit
.proc _px_blit ; u16 len, u16 addr
	; Store address to ptr1.
	sta ptr1 + 0
	stx ptr1 + 1
	
	; Store length to sreg.
	jsr popax
	sta sreg+0
	stx sreg+1
	
	txa
	beq @remainder
	
	tax
	ldy #0
	@copy_block:
		:	lda (ptr1), y
			sta PPU_VRAM_IO
			iny
			bne :-
		inc ptr1+1
		dex
		bne @copy_block
	
	@remainder:
	ldy #0
	:	cpy sreg+0
		beq @break
		lda (ptr1), y
		sta PPU_VRAM_IO
		iny
		bne :-
	@break:
	
	rts
.endproc
