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
nmi_tmp: .res 4

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

PAL0 = %00000000
PAL1 = %01010101
PAL2 = %10101010
PAL3 = %11111111
METATILE0: .byte  $00,  $80,  $82,  $84,  $86,  $8A,  $88,  $88,  $88,  $8C,  $8C,  $8C,  $8C
METATILE1: .byte  $00,  $81,  $83,  $85,  $87,  $89,  $8B,  $89,  $89,  $8D,  $8D,  $8D,  $8D
METATILE2: .byte  $00,  $90,  $92,  $94,  $96,  $98,  $98,  $9A,  $98,  $92,  $92,  $92,  $92
METATILE3: .byte  $00,  $91,  $93,  $91,  $97,  $99,  $99,  $99,  $9B,  $93,  $93,  $93,  $93
METATILE4: .byte PAL0, PAL0, PAL1, PAL2, PAL3, PAL0, PAL1, PAL2, PAL3, PAL0, PAL1, PAL2, PAL3

QUADRANT_MASK: .byte %00000011, %00001100, %00110000, %11000000

.code

.proc exec_set_metatile
	_addr = nmi_tmp + 0
	_attr = nmi_tmp + 2
	
	; Pop metatile index.
	pla
	tax
	
	; Pop and set address.
	pla
	sta _addr + 1
	sta PPU_VRAM_ADDR
	pla
	sta _addr + 0
	sta PPU_VRAM_ADDR
	
	; Write top half of block.
	lda METATILE0, x
	sta PPU_VRAM_IO
	lda METATILE1, x
	sta PPU_VRAM_IO
	
	; Increment address to the next row.
	lda _addr + 1
	sta PPU_VRAM_ADDR
	lda _addr + 0
	clc
	adc #$20
	sta PPU_VRAM_ADDR
	
	; Write bottom half.
	lda METATILE2, x
	sta PPU_VRAM_IO
	lda METATILE3, x
	sta PPU_VRAM_IO
	
	; Read back the attribute byte
	pla
	tax
	sta PPU_VRAM_ADDR
	pla
	tay
	sta PPU_VRAM_ADDR
	
	lda PPU_VRAM_IO
	lda PPU_VRAM_IO
	sta _attr
	
	; Apply the quadrant mask.
	pla
	and _attr
	sta _attr
	
	; Write back the attribute byte with the new quadrant.
	txa
	sta PPU_VRAM_ADDR
	tya
	sta PPU_VRAM_ADDR
	
	pla
	ora _attr
	sta PPU_VRAM_IO
	
	rts
.endproc

.export _px_buffer_set_metatile
.proc _px_buffer_set_metatile
	_index = 0
	_addr = ptr1
	_qmask = tmp1
	cmd_bytes = (2 + 7)
	
	; Save tile address.
	sta _addr + 0
	stx _addr + 1
	
	; Write tile address.
	ldy px_buffer_cursor
	buffer_write_ax 1
	
	; Write tile index.
	ldx px_buffer_cursor
	c_var _index
	buffer_write_arg 0
	
	; Calculate quadrant index.
	lda _addr + 0
	lsr a
	and #1
	; Set bit two on odd rows by checking bit 6 of the address byte.
	bit _addr + 0
	bvc :+
		ora #2
	:
	
	; Load the quadrant mask.
	tay
	lda QUADRANT_MASK, y
	sta _qmask
	
	; Write attribute byte address high byte.
	ldx px_buffer_cursor
	lda _addr + 1
	and #%11111100 ; Mask table address.
	ora #%00000011 ; Attribute memory start high bits.
	buffer_write_arg 3
	
	; Calculate attribute byte offset.
	lda _addr + 0
	ror _addr + 1
	ror a
	ror _addr + 1
	ror a
	tay
	and #%00000111
	sta _addr + 0
	tya
	lsr a
	lsr a
	and #%00111000
	ora _addr + 0
	
	; Write attribute byte address low byte.
	ora #%11000000 ; Attribute memory start low bits.
	buffer_write_arg 4
	
	; Write quadrant mask.
	lda _qmask
	eor #$FF
	buffer_write_arg 5
	
	; Write attribute byte.
	c_var _index
	tay
	lda METATILE4, y
	and _qmask
	buffer_write_arg 6
	
	buffer_write_func exec_set_metatile
	
	lda px_buffer_cursor
	add #cmd_bytes
	sta px_buffer_cursor

	jmp incsp1
.endproc
