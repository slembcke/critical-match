; TODO coro_resume needs to push a return catch address.

.include "zeropage.inc"
.macpack generic

.data

CORO_IP: .res 2
CORO_SP: .res 2
CORO_STACK: .res 16
CORO_STACK_END:

.code

.export _coro_start
.proc _coro_start ; coro_func func -> void
	; Subtract 1 from the function address due to how jsr/ret work.
	sub #1
	sta CORO_IP+0
	bcs :+
		dex
	:
	stx CORO_IP+1
	
	lda #>(CORO_STACK_END)
	sta CORO_SP+0
	lda #<(CORO_STACK_END)
	sta CORO_SP+1
	
	rts
.endproc

.export _coro_yield = coro_switch
.export _coro_resume = coro_switch
.proc coro_switch ; u8 value -> void
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Swap stack pointers.
	ldx sp+0
	ldy sp+1
	
	lda CORO_SP+0
	sta sp+0
	lda CORO_SP+1
	sta sp+1
	
	stx CORO_SP+0
	sty CORO_SP+1
	
	; Stash the return address.
	pla
	tax
	pla
	tay
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	; Save the old return address.
	stx CORO_IP+0
	sty CORO_IP+1
	
	; Load the return value.
	lda sreg+0
	ldx sreg+1
	
	; Return to the new address.
	rts
.endproc
