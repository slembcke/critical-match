; TODO implement context switching.

.include "zeropage.inc"
.macpack generic

.import pusha, popa
.import pushax, popax
.import addysp, subysp
.import incsp2

; What to call when resuming a coroutine that has finished.
.import _exit
CORO_ABORT = _exit

.zeropage

CORO_BUFF_PTR: .res 2

CORO_PC: .res 2 ; Yield/resume program counter - 1.
CORO_SP: .res 2 ; Yield/resume stack pointer.
CORO_S: .res 1 ; S register adjust value.

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
	sta CORO_PC+0
	lda ptr1+1
	sta CORO_PC+1
	
	; Load the return value.
	lda sreg+0
	ldx sreg+1
	
	rts
.endproc

.export _coro_init
.proc _coro_init ; coro_func func, u8 *coro_buffer, size_t buffer_size -> void
	; Stash buffer size.
	sta sreg+0
	stx sreg+1
	
	; lda CORO_BUFF_PTR+0
	; bne :+
	; lda CORO_BUFF_PTR+1
	; bne :+
	; 	jsr coro_swap_stack
	; 	jsr coro_save
	; 	jsr coro_swap_stack
	; :
	
	; Load the buffer pointer.
	jsr popax
	sta CORO_BUFF_PTR+0
	stx CORO_BUFF_PTR+1
	
	; Store the end address into the stack pointer.
	add sreg+0
	sta CORO_SP+0
	txa
	adc sreg+1
	sta CORO_SP+1
	
	; Subtract 1 from the function address due to how jsr/ret work.
	jsr popax
	sub #1
	sta CORO_PC+0
	bcs :+
		dex
	:
	stx CORO_PC+1
	
	; Push coro_catch - 1 onto the C stack in reverse byte order.
	; When it's copied onto the CPU stack it will have the right byte order.
	lda #2
	sta CORO_S
	
	jsr coro_swap_stack
	lda #>(coro_catch - 1)
	ldx #<(coro_catch - 1)
	jsr pushax
	jsr coro_save
	jmp coro_swap_stack
.endproc

; .proc coro_save
; 	; Push program counter.
; 	lda CORO_PC+0
; 	ldx CORO_PC+1
; 	jsr pushax
	
; 	; Push stack offset.
; 	lda CORO_S
; 	jsr pusha
	
; 	; Save stack pointer.
; 	ldy #0
; 	lda CORO_SP+0
; 	sta (CORO_BUFF_PTR), y
; 	iny
; 	lda CORO_SP+1
; 	sta (CORO_BUFF_PTR), y
	
; 	rts
; .endproc

; .proc coro_restore ; void *coro_buffer
; 	sta CORO_BUFF_PTR+0
; 	stx CORO_BUFF_PTR+1
	
; 	; Restore stack pointer.
; 	ldy #0
; 	lda (CORO_BUFF_PTR), y
; 	sta CORO_SP+0
; 	iny
; 	lda (CORO_BUFF_PTR), y
; 	sta CORO_SP+1
	
; 	; Pop stack offset.
; 	jsr popa
; 	sta CORO_S
	
; 	; Pop program counter.
; 	jsr popax
; 	sta CORO_PC+0
; 	stx CORO_PC+1
; .endproc

.export _coro_bind
.proc _coro_bind ; u8 *coro_buffer
	jsr coro_swap_stack
	jsr coro_save
	jsr coro_swap_stack
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
	lda CORO_PC+1
	pha
	lda CORO_PC+0
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
	lda CORO_PC+1
	pha
	lda CORO_PC+0
	pha
	
	jsr coro_swap_stack
	jmp coro_finish
.endproc

.proc coro_catch ; u16 -> u16
	; Save the resume value;
	sta sreg+0
	stx sreg+1
	
	; Push a new return address.
	lda CORO_PC+1
	pha
	lda CORO_PC+0
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
