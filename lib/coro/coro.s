; TODO coro_resume needs to push a return catch address.

.include "zeropage.inc"
.macpack generic

.data

CORO_RESUME_ADDR: .res 2
CORO_YIELD_ADDR: .res 2
CORO_SP: .res 2
CORO_STACK: .res 16
CORO_STACK_END:

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

.macro swap_bytes var_a, var_b
	lda var_a
	ldx var_b
	stx var_a
	sta var_b
.endmacro

.export _coro_start
.proc _coro_start ; coro_func func -> void
	; Subtract 1 from the function address due to how jsr/ret work.
	sub #1
	sta CORO_RESUME_ADDR+0
	bcs :+
		dex
	:
	stx CORO_RESUME_ADDR+1
	
	lda #>(CORO_STACK_END)
	sta CORO_SP+0
	lda #<(CORO_STACK_END)
	sta CORO_SP+1
	
	rts
.endproc

.export _coro_resume
.proc _coro_resume ; u8 value -> void
	; Save the resume value;
	sta sreg
	
	swap_bytes sp+0, CORO_SP+0
	swap_bytes sp+1, CORO_SP+1
	
	; Swap out the return address.
	pull_address CORO_YIELD_ADDR
	push_address CORO_RESUME_ADDR
	
	; Return the resume address.
	lda sreg
	ldx #0
	rts
.endproc

.export _coro_yield
.proc _coro_yield ; void -> u8
	; Save the yield value.
	sta sreg
	
	swap_bytes sp+0, CORO_SP+0
	swap_bytes sp+1, CORO_SP+1
	
	; Swap out the return address.
	pull_address CORO_RESUME_ADDR
	push_address CORO_YIELD_ADDR
	
	; Return the yield value.
	lda sreg
	ldx #0
	rts
.endproc
