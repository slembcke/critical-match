.include "zeropage.inc"
.include "pixler.inc"

.importzp px_ctrl
.importzp PX_buffer

.import incsp1
.import METATILE0, METATILE1, METATILE2, METATILE3, METATILE4

.macpack generic

.zeropage

.exportzp px_buffer_cursor
px_buffer_cursor: .byte 0

.exportzp px_nmi_tmp
px_nmi_tmp: .res 4

.code

; Restores the stack and jumps to the caller of px_buffer_exec
.proc exec_terminator
	pla
	tax
	txs
	rts
.endproc

.export _px_buffer_clear
.proc _px_buffer_clear
	ldx #0
	stx px_buffer_cursor
	
	rts
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

; Execute an update buffer stored in stack.
; Idea based on: http://forums.nesdev.com/viewtopic.php?f=2&t=16969
.export _px_buffer_exec
.proc _px_buffer_exec
	tsx
	txa
	
	ldx px_buffer_cursor
	px_buffer_write_arg 0
	px_buffer_write_func exec_terminator
	
	jsr _px_buffer_clear
	
	; Manipulate the stack to jump to ($0100)
	; This is the first display list buffer func.
	ldx #$FF
	txs
	rts
.endproc

.proc exec_inc_h
	lda px_ctrl
	and #(~$04 & $FF)
	sta px_ctrl
	sta PPU_CTRL
	rts
.endproc

.proc exec_inc_v
	lda px_ctrl
	ora #$04
	sta px_ctrl
	sta PPU_CTRL
	rts
.endproc

.export _px_buffer_inc
.proc _px_buffer_inc ; pxInc direction
	ldx px_buffer_cursor
	
	cmp #0
	beq @horiz
		px_buffer_write_func exec_inc_v
		jmp :+
	@horiz:
		px_buffer_write_func exec_inc_h
	:
	
	inx
	inx
	stx px_buffer_cursor
	
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
	ldy #(_len)
	lda (sp), y
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
	
	jmp incsp1
.endproc

.proc exec_set_color
	lda #>PPU_PAL0
	sta PPU_VRAM_ADDR
	pla
	sta PPU_VRAM_ADDR
	
	pla
	sta PPU_VRAM_IO
	
	rts
.endproc

.export _px_buffer_set_color
.proc _px_buffer_set_color ; u8 idx, u8 color
	_idx = 0
	; _color = a
	cmd_bytes = (2 + 2)
	
	; Write color.
	ldx px_buffer_cursor
	px_buffer_write_arg 1
	
	; Write idx
	ldy #(_idx)
	lda (sp), y
	px_buffer_write_arg 0
	
	px_buffer_write_func exec_set_color
	
	lda px_buffer_cursor
	add #cmd_bytes
	sta px_buffer_cursor
	
	jmp incsp1
.endproc
