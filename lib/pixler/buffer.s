.macpack generic

.include "zeropage.inc"
.import popa

.include "pixler.inc"
.include "buffer.inc"

.importzp PX_buffer

.zeropage

.exportzp px_nmi_tmp
px_nmi_tmp: .res 4

.code

.export _px_buffer_clear
.proc _px_buffer_clear
	ldx #0
	stx px_buffer_cursor
	
	rts
.endproc

; Execute an update buffer stored in stack.
; Idea based on: http://forums.nesdev.com/viewtopic.php?f=2&t=16969
.export _px_buffer_exec
.proc _px_buffer_exec
	tsx
	txa
	
	ldx px_buffer_cursor
	px_buffer_write_arg 0
	px_buffer_write_func terminator
	
	jsr _px_buffer_clear
	
	; Manipulate the stack to jump to ($0100)
	; This is the first display list buffer func.
	ldx #$FF
	txs
	rts
	
	; Restores the stack and jumps to the caller of px_buffer_exec
	.proc terminator
		pla
		tax
		txs
		rts
	.endproc
.endproc

.proc exec_data
	pla
	sta PPU_VRAM_ADDR
	pla
	sta PPU_VRAM_ADDR
	
	pla
	tax
	:	pla
		sta PPU_VRAM_IO
		dex
		bne :-
	
	rts
.endproc

.export _px_buffer_data
.proc _px_buffer_data ; u8 len, u16 addr
	_len = 0
	; _addr = x|a
	cmd_bytes = (2 + 3)
	
	ldy px_buffer_cursor
	px_buffer_write_ax 0
	
	ldx px_buffer_cursor
	px_buffer_write_func exec_data
	
	; Write len.
	jsr popa
	px_buffer_write_arg 2
	
	; Increment cursor
	clc
	adc #cmd_bytes
	adc px_buffer_cursor
	sta px_buffer_cursor
	
	; Set PX_buffer
	txa
	adc #cmd_bytes
	sta PX_buffer + 0
	lda #$01
	sta PX_buffer + 1
	
	rts
.endproc
