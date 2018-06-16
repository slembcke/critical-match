.include "zeropage.inc"
.include "pixler.inc"

.importzp px_ctrl
.importzp PX_buffer

.import incsp1

.export px_buffer_exec
.export _px_buffer_inc
.export _px_buffer_data
.export _px_buffer_set_color

.macpack generic

.zeropage

px_buffer_cursor: .byte 0
px_ptr: .addr $0000

.code

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

.proc exec_set_color
	lda #>PPU_PAL0
	sta PPU_VRAM_ADDR
	pla
	sta PPU_VRAM_ADDR
	
	pla
	sta PPU_VRAM_IO
	
	rts
.endproc

; Restores the stack and jumps to the caller of px_buffer_exec
.proc exec_terminator
	pla
	tax
	txs
	rts
.endproc

.macro buffer_write_func f
	lda #<(f - 1)
	sta $0100, x
	lda #>(f - 1)
	sta $0101, x
.endmacro

.macro buffer_write_arg idx, value
	.scope
		.ifnblank value
			lda #value
		.endif
		
		addr = $0102 + idx
		sta addr, x
	.endscope
.endmacro

.macro buffer_write_ax idx
	.scope
		sta $0103 + idx, y
		txa
		sta $0102 + idx, y
	.endscope
.endmacro

; Execute an update buffer stored in stack.
; Idea based on: http://forums.nesdev.com/viewtopic.php?f=2&t=16969
.proc px_buffer_exec
	tsx
	txa
	
	ldx px_buffer_cursor
	buffer_write_arg 0
	buffer_write_func exec_terminator
	
	; Reset buffer cursor.
	ldx #0
	stx px_buffer_cursor
	
	; Manipulate the stack to jump to ($0100)
	; This is the first display list buffer func.
	ldx #$FF
	txs
	rts
.endproc

.proc _px_buffer_inc ; pxInc direction
	ldx px_buffer_cursor
	
	cmp #0
	beq @horiz
		buffer_write_func exec_inc_v
		jmp :+
	@horiz:
		buffer_write_func exec_inc_h
	:
	
	inx
	inx
	stx px_buffer_cursor
	
	rts
.endproc

.proc _px_buffer_data ; u8 len, u16 addr
	_len = 0
	; _addr = x|a
	cmd_bytes = (2 + 3)
	
	ldy px_buffer_cursor
	buffer_write_ax 0
	
	ldx px_buffer_cursor
	buffer_write_func exec_data
	
	; Write len.
	c_var _len
	buffer_write_arg 2
	
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

.proc _px_buffer_set_color ; u8 idx, u8 color
	_idx = 0
	; _color = a
	cmd_bytes = (2 + 2)
	
	; Write color.
	ldx px_buffer_cursor
	buffer_write_arg 1
	
	; Write idx
	c_var _idx
	buffer_write_arg 0
	
	buffer_write_func exec_set_color
	
	lda px_buffer_cursor
	add #cmd_bytes
	sta px_buffer_cursor
	
	jmp incsp1
.endproc

.rodata ; TODO move this somewhere else.

PAL0 = $00
PAL1 = $55
PAL2 = $AA
PAL3 = $FF
METATILE0: .byte  $00,  $94,  $94,  $14,  $14
METATILE1: .byte  $00,  $92,  $92,  $12,  $12
METATILE2: .byte  $00,  $91,  $91,  $11,  $11
METATILE3: .byte  $00,  $88,  $88,  $08,  $08
METATILE4: .byte PAL0, PAL0, PAL1, PAL2, PAL3

.code

.proc exec_set_metatile
	; Pop metatile index.
	pla
	tax
	
	; Pop and set address.
	pla
	sta px_ptr + 1
	sta PPU_VRAM_ADDR
	pla
	sta px_ptr + 0
	sta PPU_VRAM_ADDR
	
	; Write top half of block.
	lda METATILE0, x
	sta PPU_VRAM_IO
	lda METATILE1, x
	sta PPU_VRAM_IO
	
	; Increment addr to bottom half.
	lda px_ptr + 0
	clc
	adc #$20
	tay
	lda px_ptr + 1
	adc #0
	
	sta PPU_VRAM_ADDR
	tya
	sta PPU_VRAM_ADDR
	
	; Write bottom half.
	lda METATILE2, x
	sta PPU_VRAM_IO
	lda METATILE3, x
	sta PPU_VRAM_IO
	
	pla
	sta PPU_VRAM_ADDR
	pla
	; lda px_ptr + 0
	; lsr a
	; lsr a
	; and #$07
	; ora #$C0
	sta PPU_VRAM_ADDR
	pla
	sta PPU_VRAM_IO
	
	rts
.endproc

.export _px_buffer_set_metatile
.proc _px_buffer_set_metatile
	cmd_bytes = (2 + 6)
	
	; Save tile address.
	sta ptr1 + 0
	stx ptr1 + 1
	
	; Write tile address.
	ldy px_buffer_cursor
	buffer_write_ax 1
	
	; Write tile index.
	lda #2
	ldx px_buffer_cursor
	buffer_write_arg 0
	
	; Write attribute byte address high byte.
	ldx px_buffer_cursor
	lda ptr1 + 1
	and #$2C ; Mask table address.
	ora #$03 ; Attribute memory start high bits.
	buffer_write_arg 3
	
	; Calculate attribute byte offset.
	lda ptr1 + 0
	ror ptr1 + 1
	ror a
	ror ptr1 + 1
	ror a
	tay
	and #$07
	sta ptr1 + 0
	tya
	lsr
	lsr
	ora ptr1 + 0
	
	; Write attribute byte 
	ora #$C0 ; Attribute memory start low bits.
	buffer_write_arg 4
	
	; Write attribute byte..
	lda #$FF
	buffer_write_arg 5
	
	buffer_write_func exec_set_metatile
	
	lda px_buffer_cursor
	add #cmd_bytes
	sta px_buffer_cursor

	rts
.endproc
