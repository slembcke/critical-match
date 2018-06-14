.include "zeropage.inc"
.include "pixler.inc"

.importzp px_ctrl

.import incsp1

.export px_blit_pages, __px_blit_pages = px_blit_pages
.export _px_inc
.export _px_fill

.proc px_blit_pages ; a = base_page, x = count
	ldy #0
	sty ptr1 + 0
	sta ptr1 + 1
	
	:	lda (ptr1), y
		sta PPU_VRAM_IO
		iny
		bne :-
	inc ptr1 + 1
	dex
	bne :-
	
	rts
.endproc

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

.proc _px_fill ; u8 chr, u16 len
	_chr = 0
	; _len = x|a
	
	; TODO high byte in x is ignored
	sta sreg
	
	c_var _chr
	ldy sreg
	: sta PPU_VRAM_IO
		dey
		bne :-
	
	jmp incsp1
.endproc
