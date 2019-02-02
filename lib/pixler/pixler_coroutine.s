; TODO Save CORO_BUFF_PTR so coroutines can call coroutines?

.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax, popax
.import addysp, subysp

; Error handler to call when resuming a coroutine that has finished.
.import _exit
CORO_ERROR = _exit

.zeropage

; Pointer to the currently running coroutine.
CORO_BUFF_PTR: .res 2

; struct {
; 	// Current top of the C stack
; 	u8 *c_stack_ptr;
; 	// # of C stack bytes to be transferred to the CPU stack.
; 	u8 cpu_stack_bytes
	
; 	// C Stack buffer.
; 	// At a minimum contains the coroutine resume address,
; 	// and the address function that handles the coroutine completion.
; 	u8 c_stack_buff[];
; }

.code

; Swap the stack stored by the coroutine with the one from the C runtime.
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

; During initialization:
; * The pointer to the top of the stack is calculated and stored.
; * The coroutine start address is calculated and pushed onto the coroutine's stack.
; * The "catch" address is pushed onto the stack.
; * The number of stack bytes to be copied from the coroutine's stack to the CPU stack is stored.
;   At init time, this value is always 2 for the "catch" function jump address.
.export _px_coro_init
.proc _px_coro_init ; naco_func func, u8 *naco_buffer, size_t buffer_size -> void
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

; Shared by resume/yield/catch
.proc naco_finish
	value = sreg
	
	; Pop the jump address from the current stack.
	jsr popax
	pha
	txa
	pha
	
	; Load the return value.
	lda value+0
	ldx value+1
	
	rts
.endproc

; Resume a coroutine.
.export _px_coro_resume
.proc _px_coro_resume ; void *naco_buffer, u16 value -> u16
	value = sreg
	stack_bytes = tmp1
	
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
	
	; Load # stack bytes to copy.
	ldy #2
	lda (CORO_BUFF_PTR), y
	sta stack_bytes
	
	; Save the stack register.
	tsx
	txa
	sta (CORO_BUFF_PTR), y
	
	ldy #0
	:	lda (sp), y
		pha
		iny
		cpy stack_bytes
		bne :-
	jsr addysp
	
	jmp naco_finish
.endproc

; Yield from a coroutine back to the main thread.
.export _px_coro_yield
.proc _px_coro_yield ; u16 value -> u16
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
	tsx
	txa
	ldy #2
	clc ; For 2's complement.
	sbc (CORO_BUFF_PTR), y
	eor #$FF
	
	; Save the value back to the coroutine buffer.
	sta (CORO_BUFF_PTR), y
	
	; Copy bytes from the CPU stack to the coroutine stock.
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

; This is called when a coroutine's body function returns.
; It handles the special case, and sets CORO_ERROR to be called if
; the coroutine is accidentally resumed after completing.
.proc naco_catch ; u16 -> u16
	value = sreg
	
	; Save the resume value;
	sta value+0
	stx value+1
	
	; Push error func.
	lda #>(CORO_ERROR - 1)
	ldx #<(CORO_ERROR - 1)
	jsr pushax
	
	; Zero out the stack offset.
	lda #0
	ldy #2
	sta (CORO_BUFF_PTR), y
	
	jsr naco_swap_sp
	jmp naco_finish
.endproc
