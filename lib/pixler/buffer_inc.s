.include "pixler.inc"
.include "buffer.inc"

.importzp px_ctrl

.export _px_buffer_inc_h
.proc _px_buffer_inc_h
	ldy px_buffer_cursor
	px_buffer_write_func exec
	
	iny
	iny
	sty px_buffer_cursor
	
	rts
	
	.proc exec
		lda px_ctrl
		and #(~$04 & $FF)
		sta px_ctrl
		sta PPU_CTRL
		
		rts
	.endproc
.endproc

.export _px_buffer_inc_v
.proc _px_buffer_inc_v
	ldy px_buffer_cursor
	px_buffer_write_func exec
	
	iny
	iny
	sty px_buffer_cursor
	
	rts
	
	.proc exec
		lda px_ctrl
		ora #$04
		sta px_ctrl
		sta PPU_CTRL
		
		rts
	.endproc
.endproc
