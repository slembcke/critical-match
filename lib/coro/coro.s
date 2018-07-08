; TODO coro_resume needs to push a return catch address.

.export _resume_addr = RESUME_ADDR
.export _yield_addr = YIELD_ADDR

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
.proc _coro_start ; coro_func func
	; Subtract 1 from the function address due to how jsr/ret work.
	sub #1
	sta RESUME_ADDR+0
	sbc #0
	stx RESUME_ADDR+1
	
	rts
.endproc

.export _coro_resume
.proc _coro_resume
	pull_address YIELD_ADDR
	
	push_address RESUME_ADDR
	rts
.endproc

.export _coro_yield
.proc _coro_yield
	pull_address RESUME_ADDR
	
	push_address YIELD_ADDR
	rts
.endproc
