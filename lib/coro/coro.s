.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax
.import addysp, subysp
.import _exit

.data

CORO_ABORT = _exit

CORO_IP: .res 2 ; Yield/resume instruction pointer - 1
CORO_SP: .res 2 ; Yield/resume stack pointer.
CORO_S: .res 1 ; S register adjust value.
CORO_STACK: .res 16
CORO_STACK_END:

.export CORO_STACK, CORO_STACK_END, coro_catch

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

.proc coro_finish
	; Save the old return address.
	lda ptr1+0
	sta CORO_IP+0
	lda ptr1+1
	sta CORO_IP+1
	
	; Load the return value.
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
	
	lda #<(CORO_STACK_END)
	sta CORO_SP+0
	lda #>(CORO_STACK_END)
	sta CORO_SP+1
	
	lda #2
	sta CORO_S
	
	; Push coro_catch - 1 onto the C stack in reverse order.
	; When it's copied onto the CPU stack it will have the right byte order.
	jsr coro_swap_stack
	lda #>(coro_catch - 1)
	ldx #<(coro_catch - 1)
	jsr pushax
	jmp coro_swap_stack
.endproc

.export _coro_resume
.proc _coro_resume ; u16 -> u16
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Stash the return address.
	pla
	sta ptr1+0
	pla
	sta ptr1+1
	
	jsr coro_swap_stack
	
	; Stash the stack register.
	tsx
	
	lda CORO_S
	beq @skip_copy
	
	ldy #0
	:	lda (sp), y
		pha
		iny
		cpy CORO_S
		bne :-
	jsr addysp
	@skip_copy:
	
	; Save the old stack register value.
	stx CORO_S
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	jmp coro_finish
.endproc

.export _coro_yield
.proc _coro_yield ; 16 -> u16
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
	eor #$FF
	sta CORO_S
	tay
	
	cmp #0
	beq @skip_copy
	
	jsr subysp
	:	dey
		pla
		sta (sp), y
		cpy #0
		bne :-
	@skip_copy:
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	jsr coro_swap_stack
	jmp coro_finish
.endproc

.proc coro_catch ; u16 -> u16
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Push a new return address.
	lda CORO_IP+1
	pha
	lda CORO_IP+0
	pha
	
	; Invalidate the resume stack/address.
	lda #0
	sta CORO_S
	
	lda #<(CORO_ABORT - 1)
	sta ptr1+0
	lda #>(CORO_ABORT - 1)
	sta ptr1+1
	
	jsr coro_swap_stack
	jmp coro_finish
.endproc
