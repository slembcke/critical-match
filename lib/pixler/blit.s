.include "zeropage.inc"
.include "pixler.inc"

.importzp px_ctrl

.import incsp2

.export _px_inc
.proc _px_inc ; u8 direction
	cmp #0
	beq @horiz
		lda px_ctrl
		ora #$04
		jmp :+
	@horiz:
		lda px_ctrl
		and #(~$04 & $FF)
	:
	
	sta px_ctrl
	sta PPU_CTRL
	
	rts
.endproc

.export _px_fill
.proc _px_fill ; u16 len, u8 chr
	_len = 0
	; _chr = a
	
	tax
	ldy #(_len+1)
	lda (sp), y
	beq @remainder

	tay
	txa
	ldx #0
	:
		:	sta PPU_VRAM_IO
			inx
			bne :-
		dey
		bne :--
	tax

	@remainder:
	ldy #(_len+0)
	lda (sp), y
	tay
	txa
	: sta PPU_VRAM_IO
		dey
		bne :-
	
	jmp incsp2
.endproc

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
