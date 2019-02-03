.include "pixler.inc"

.importzp px_ctrl

.export _px_inc_h
.proc _px_inc_h
	lda px_ctrl
	and #(~$04 & $FF)
	
	; TODO bra?
	jmp px_inc_finish
.endproc

.export _px_inc_v
.proc _px_inc_v
	lda px_ctrl
	ora #$04
	
	bne px_inc_finish ; bra
.endproc


.proc px_inc_finish
	sta px_ctrl
	sta PPU_CTRL
	
	rts
.endproc
