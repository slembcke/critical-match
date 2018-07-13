; TODO implement context switching.

.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax, popax
.import addysp, subysp
.import incsp2

; What to call when resuming a coroutine that has finished.
.import _abort
CORO_ABORT = _abort

.data

; CORO_BUFF_PTR: .res 2
CORO_BUFF_PTR = regbank

CORO_S: .res 1 ; S register adjust value.

.code

.proc coro_swap_sp
	ldy #0
	lda (CORO_BUFF_PTR), y
	ldx sp+0
	sta sp+0
	txa
	sta (CORO_BUFF_PTR), y
	
	iny
	lda (CORO_BUFF_PTR), y
	ldx sp+1
	sta sp+1
	txa
	sta (CORO_BUFF_PTR), y
	
	rts
.endproc

.export _coro_init
.proc _coro_init ; coro_func func, u8 *coro_buffer, size_t buffer_size -> void
	func = ptr1
	size = sreg
	
	; Stash buffer size.
	sta size+0
	stx size+1
	
	; Load the buffer pointer.
	jsr popax
	sta CORO_BUFF_PTR+0
	stx CORO_BUFF_PTR+1
	
	; Store the end address into the stack pointer.
	add size+0
	ldy #0
	sta (CORO_BUFF_PTR), y
	txa
	adc size+1
	iny
	sta (CORO_BUFF_PTR), y
	
	; Subtract 1 from the function address due to how jsr/ret work.
	jsr popax
	sub #1
	sta func+0
	bcs :+
		dex
	:
	stx func+1
	
	jsr coro_swap_sp
	
	lda func+1
	ldx func+0
	jsr pushax
	
	lda #>(coro_catch - 1)
	ldx #<(coro_catch - 1)
	jsr pushax
	
	lda #2
	sta CORO_S ; TODO
	
	; Restore the stack.
	jmp coro_swap_sp
.endproc

.export _coro_resume
.proc _coro_resume ; u16 -> u16
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Push the yield address to the caller's stack.
	pla
	tax
	pla
	jsr pushax
	
	jsr coro_swap_sp
	
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
	
	; Pop the resume addres from the coroutine stack.
	jsr popax
	pha
	txa
	pha
	
	; Load the return value.
	lda sreg+0
	ldx sreg+1
	
	rts
.endproc

.export _coro_yield
.proc _coro_yield ; 16 -> u16
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Push the resume address onto the coroutine's stack.
	pla
	tax
	pla
	jsr pushax
	
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
	
	
	jsr coro_swap_sp
	
	jsr popax
	pha
	txa
	pha
	
	; Load the return value.
	lda sreg+0
	ldx sreg+1
	
	rts
.endproc

.proc coro_catch ; u16 -> u16
	jmp _abort
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Invalidate the resume stack/address.
	lda #0
	sta CORO_S
	
	lda #<(CORO_ABORT - 1)
	sta ptr1+0
	lda #>(CORO_ABORT - 1)
	sta ptr1+1
	
	jsr coro_swap_sp
	
	jsr popax
	pha
	txa
	pha
	
	; Load the return value.
	lda sreg+0
	ldx sreg+1
	
	rts
.endproc
