.include "zeropage.inc"
.include "pixler.inc"

.importzp px_ctrl

.import incsp1
.import incsp2

.export px_blit_pages, __px_blit_pages = px_blit_pages
.export _px_inc
.export _px_fill
.export _px_blit

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

.proc _px_fill ; u16 len, u8 chr
	_len = 0
	; _chr = a
	
	tax
	c_var _len + 1
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
	c_var _len
	tay
	txa
	: sta PPU_VRAM_IO
		dey
		bne :-
	
	jmp incsp1
.endproc

; static void px_blit(u8 *mem, u16 len){
; 	for(; len != 0; --len) PPU.vram.data = *(mem++);
; }

.proc _px_blit ; 16 len, u16 addr
	_len = 0
	; _addr = x|a
	
	sta ptr1 + 0
	stx ptr1 + 1
	
	c_var _len + 0
	sta tmp1 + 0
	; c_var _len + 1
	; sta tmp1 + 1
	; TODO high byte is ignored
	
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
