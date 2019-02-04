.macpack generic

.import popa

.include "pixler.inc"
.include "buffer.inc"

.importzp PX_buffer

.code

.export _px_buffer_data
.proc _px_buffer_data ; u8 len, u16 addr
	_len = 0
	; _addr = x|a
	cmd_bytes = (2 + 3) ; func + len + addr
	
	ldy px_buffer_cursor
	px_buffer_write_ax 0
	px_buffer_write_func exec
	
	; Write len.
	jsr popa
	ldy px_buffer_cursor
	px_buffer_write_arg 2
	
	; Increment cursor, a == len
	add #cmd_bytes
	adc px_buffer_cursor
	sta px_buffer_cursor
	
	; Set PX_buffer
	tya
	adc #cmd_bytes
	sta PX_buffer + 0
	lda #$01
	sta PX_buffer + 1
	
	rts
	
	.proc exec
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
.endproc
