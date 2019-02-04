.include "pixler.inc"
.include "buffer.inc"

.code

.export _px_buffer_clear
.proc _px_buffer_clear
	ldy #0
	sty px_buffer_cursor
	
	rts
.endproc

; Execute an update buffer stored in stack.
; Idea based on: http://forums.nesdev.com/viewtopic.php?f=2&t=16969
.export _px_buffer_exec
.proc _px_buffer_exec
	tsx
	txa
	
	ldy px_buffer_cursor
	px_buffer_write_arg 0
	px_buffer_write_func terminator
	
	jsr _px_buffer_clear
	
	; Manipulate the stack to jump to ($0100)
	; This is the first display list buffer func.
	ldx #$FF
	txs
	rts
	
	; Restores the stack and jumps to the caller of px_buffer_exec
	.proc terminator
		pla
		tax
		txs
		rts
	.endproc
.endproc
