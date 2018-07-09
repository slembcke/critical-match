; TODO coro_resume needs to push a return catch address.

.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax
.import addysp, subysp
.import _abort

.data

CORO_IP: .res 2 ; Yield/resume instruction pointer - 1
CORO_SP: .res 2 ; Yield/resume stack pointer.
CORO_S: .res 1 ; S register adjust value.
CORO_STACK: .res 16
CORO_STACK_END:

.export _stack_offset = CORO_S

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
	
	lda #2
	sta CORO_S
	
	jmp coro_swap_stack
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
	
	; Stash the stack register.
	tsx
	
	ldy #0
	: cpy CORO_S
		beq :+
		lda (sp), y
		pha
		iny
		jmp :-
	:
	jsr addysp
	
	; Save the old stack register value.
	stx CORO_S
	
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
	
	; Calculate stack offset. -(s - CORO_S)
	tsx
	txa
	clc
	sbc CORO_S
	eor $FF
	tay
	sty CORO_S
	
	; Stack offset will be at least 2 for the coro_catch address.
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
	
	lda #0
	sta CORO_S
	
	jsr coro_swap_stack
	jmp coro_ret_sreg
.endproc
