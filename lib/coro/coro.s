; TODO coro_resume needs to push a return catch address.

.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax
.import addysp, subysp
.import _abort

.data

CORO_IP: .res 2
CORO_SP: .res 2
CORO_STACK: .res 16
CORO_STACK_END:

.code

.proc coro_swap_stack
	ldx sp+0
	ldy sp+1
	
	lda CORO_SP+0
	sta sp+0
	lda CORO_SP+1
	sta sp+1
	
	stx CORO_SP+0
	sty CORO_SP+1
	
	rts
.endproc

.proc coro_ret_sreg
	lda sreg+0
	ldx sreg+1
	
	rts
.endproc

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
	
	jsr coro_swap_stack
	lda #>(coro_catch - 1)
	ldx #<(coro_catch - 1)
	jsr pushax
	
	jsr coro_swap_stack
	
	rts
.endproc

.export _coro_resume
.proc _coro_resume ; u8 value -> void
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	jsr coro_swap_stack
	
	; Stash the return address.
	pla
	sta ptr1+0
	pla
	sta ptr1+1
	
	ldy #0
	: lda (sp), y
		pha
		iny
		cpy #2
		bne :-
	jsr addysp
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	; Save the old return address.
	lda ptr1+0
	sta CORO_IP+0
	lda ptr1+1
	sta CORO_IP+1
	
	jmp coro_ret_sreg
.endproc

.export _coro_yield
.proc _coro_yield ; u8 value -> void
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Stash the return address.
	pla
	sta ptr1+0
	pla
	sta ptr1+1
	
	ldy #2
	jsr subysp
	: dey
		pla
		sta (sp), y
		cpy #0
		bne :-
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	; Save the old return address.
	lda ptr1+0
	sta CORO_IP+0
	lda ptr1+1
	sta CORO_IP+1
	
	jsr coro_swap_stack
	jmp coro_ret_sreg
.endproc

.proc coro_catch ; u8 value -> void
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	; Invalidate the resume address.
	lda #<(_abort - 1)
	sta CORO_IP+0
	lda #>(_abort - 1)
	sta CORO_IP+1
	
	jsr coro_swap_stack
	jmp coro_ret_sreg
.endproc
