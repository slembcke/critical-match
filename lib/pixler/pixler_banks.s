.include "zeropage.inc"
.include "pixler.inc"

.export px_bank_select, _px_bank_select

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

.rodata

.export _neschar_inc
_neschar_inc:
	.include "neschar.inc"

.export _gfx_sheet1_lz4chr
_gfx_sheet1_lz4chr:
	.incbin "gfx/sheet1.lz4chr", 8

.export _gfx_squidman_lz4chr
_gfx_squidman_lz4chr:
	.incbin "gfx/squidman.lz4chr", 8

.segment "PRG0"
.segment "PRG1"
.segment "PRG2"
.segment "PRG3"
.segment "PRG4"
.segment "PRG5"
.segment "PRG6"
