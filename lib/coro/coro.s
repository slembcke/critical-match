; TODO coro_resume needs to push a return catch address.

.include "zeropage.inc"
.macpack generic

.data

RESUME_ADDR: .res 2
YIELD_ADDR: .res 2

.code

.macro pull_address var
	pla
	sta var+0
	pla
	sta var+1
.endmacro

.macro push_address var
	lda var+1
	pha
	lda var+0
	pha
.endmacro

.export _coro_start
.proc _coro_start ; coro_func func -> void
	; Subtract 1 from the function address due to how jsr/ret work.
	sub #1
	sta RESUME_ADDR+0
	bcs :+
		dex
	:
	stx RESUME_ADDR+1
	
	rts
.endproc

.export _coro_resume
.proc _coro_resume ; u8 value -> void
	; Save the resume value;
	sta sreg
	
	; Swap out the return address.
	pull_address YIELD_ADDR
	push_address RESUME_ADDR
	
	; Return the resume address.
	lda sreg
	rts
.endproc

.export _coro_yield
.proc _coro_yield ; void -> u8
	; Save the yield value.
	sta sreg
	
	; Swap out the return address.
	pull_address RESUME_ADDR
	push_address YIELD_ADDR
	
	; Return the yield value.
	lda sreg
	rts
.endproc
