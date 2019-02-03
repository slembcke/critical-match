.include "zeropage.inc"
.include "pixler.inc"

.import incsp2

.export _px_blit
.proc _px_blit ; u16 len, u16 addr
	_len = 0
	; _addr = x|a
	
	sta ptr1 + 0
	stx ptr1 + 1
	
	ldy #(_len+1)
	lda (sp), y
	beq @remainder

	tax
	ldy #0
	:
		:	lda (ptr1), y
			sta PPU_VRAM_IO
			iny
			bne :-
		inc ptr1 + 1
		dex
		bne :--

	@remainder:
	ldy #(_len+0)
	lda (sp), y
	sta tmp1 + 0
	
	ldy #0
	:	cpy tmp1 + 0
		beq :+
		lda (ptr1), y
		sta PPU_VRAM_IO
		iny
		jmp :-
	:
	
	jmp incsp2
.endproc
