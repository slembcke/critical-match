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

.segment "PRG0"
.segment "PRG1"
.segment "PRG2"
.segment "PRG3"
.segment "PRG4"
.segment "PRG5"
.segment "PRG6"
