; TODO Save CORO_BUFF_PTR so coroutines can call coroutines?

.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax, popax
.import addysp, subysp

; Error handler to call when resuming a coroutine that has finished.
.import _exit
CORO_ABORT = _exit

.zeropage

; Pointer to the currently running coroutine.
; TODO document buffer layout.
CORO_BUFF_PTR: .res 2

.code

.proc naco_swap_sp
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

.export _naco_init
.proc _naco_init ; naco_func func, u8 *naco_buffer, size_t buffer_size -> void
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
	
	jsr naco_swap_sp
	
	lda func+1
	ldx func+0
	jsr pushax
	
	lda #>(naco_catch - 1)
	ldx #<(naco_catch - 1)
	jsr pushax
	
	lda #2
	tay
	sta (CORO_BUFF_PTR), y
	
	; Restore the stack.
	jmp naco_swap_sp
.endproc

.proc naco_finish
	value = sreg
	
	; Pop the resume address from the coroutine stack.
	jsr popax
	pha
	txa
	pha
	
	; Load the return value.
	lda value+0
	ldx value+1
	
	rts
.endproc

.export _naco_resume
.proc _naco_resume ; void *naco_buffer, u16 value -> u16
	value = sreg
	tmp = tmp1
	
	; Save the resume value;
	sta value+0
	stx value+1
	
	jsr popax
	sta CORO_BUFF_PTR+0
	stx CORO_BUFF_PTR+1
	
	; Push the yield address to the caller's stack.
	pla
	tax
	pla
	jsr pushax
	
	jsr naco_swap_sp
	
	; Stash the stack register.
	tsx
	
	ldy #2
	lda (CORO_BUFF_PTR), y
	sta tmp
	ldy #0
	:	lda (sp), y
		pha
		iny
		cpy tmp
		bne :-
	jsr addysp
	
	; Save the old stack register value.
	ldy #2
	txa
	sta (CORO_BUFF_PTR), y
	
	jmp naco_finish
.endproc

.export _naco_yield
.proc _naco_yield ; u16 value -> u16
	value = sreg
	
	; Save the resume value;
	sta value+0
	stx value+1
	
	; Push the resume address onto the coroutine's stack.
	pla
	tax
	pla
	jsr pushax
	
	; Calculate stack offset. -(s - stack_offset)
	clc
	tsx
	txa
	ldy #2
	sbc (CORO_BUFF_PTR), y
	eor #$FF
	sta (CORO_BUFF_PTR), y
	tay
	
	jsr subysp
	:	dey
		pla
		sta (sp), y
		cpy #0
		bne :-
	
	jsr naco_swap_sp
	jmp naco_finish
.endproc

.proc naco_catch ; u16 -> u16
	value = sreg
	
	; Save the resume value;
	sta value+0
	stx value+1
	
	; Push error func.
	lda #>(CORO_ABORT - 1)
	ldx #<(CORO_ABORT - 1)
	jsr pushax
	
	; Zero out the stack offset.
	lda #0
	ldy #2
	sta (CORO_BUFF_PTR), y
	
	jsr naco_swap_sp
	jmp naco_finish
.endproc
