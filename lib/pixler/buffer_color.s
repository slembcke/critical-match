.macpack generic

.import popa

.include "pixler.inc"
.include "buffer.inc"

.export _px_buffer_set_color
.proc _px_buffer_set_color ; u8 idx, u8 color
	_idx = 0
	; _color = a
	cmd_bytes = (2 + 2)
	
	; Write color.
	ldy px_buffer_cursor
	px_buffer_write_arg 1
	
	; Write idx
	jsr popa
	ldy px_buffer_cursor
	px_buffer_write_arg 0

	px_buffer_write_func exec
	
	tya
	add #cmd_bytes
	sta px_buffer_cursor
	
	rts
	
	.proc exec
		lda #>PPU_PAL0
		sta PPU_VRAM_ADDR
		pla
		sta PPU_VRAM_ADDR
		
		pla
		sta PPU_VRAM_IO
		
		rts
	.endproc
.endproc
