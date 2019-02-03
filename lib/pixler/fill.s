.macpack generic

.include "zeropage.inc"
.include "pixler.inc"

.import popax

.export _px_fill
.proc _px_fill ; u16 len, u8 chr
	pha ; Push chr.
	
	; Store length in y/sreg+1
	; Increment both bytes for shorter loop conditions.
	jsr popax
	tay
	iny
	inx
	stx sreg+1
	
	; Pop chr.
	pla
	tax
	jmp @loop_start
	
	@loop:
		stx PPU_VRAM_IO
		@loop_start:
		dey
		bne @loop
	
	dec sreg+1
	bne @loop
	
	rts
.endproc
