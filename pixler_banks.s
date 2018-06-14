.include "zeropage.inc"
.include "pixler.inc"

.export px_bank_select, _px_bank_select
.export px_blit_pages, __px_blit_pages = px_blit_pages

.rodata
	bank_selector: .byte 0, 1, 2, 3, 4, 5, 6, 7

.code

.proc px_bank_select
  lda bank_selector, y
  sta bank_selector, y
	
  rts
.endproc

.proc _px_bank_select ; u8 bank
	and #$07
	tay
	
	jmp px_bank_select
.endproc

.proc px_blit_pages ; a = base_page, x = count
	ldy #0
	sty ptr1 + 0
	sta ptr1 + 1
	
	:		lda (sreg), y
			sta PPU_VRAM_IO
			iny
			bne :-
		inc sreg + 1
		dex
		bne :-
	
	rts
.endproc

.segment "PRG0"
	.align $100
	.include "neschar.inc"

.segment "PRG1"
.segment "PRG2"
.segment "PRG3"
.segment "PRG4"
.segment "PRG5"
.segment "PRG6"
