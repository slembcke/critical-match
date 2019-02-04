.include "zeropage.inc"
.include "pixler/pixler.inc"
.include "pixler/buffer.inc"

.macpack generic

.importzp px_nmi_tmp
.importzp px_buffer_cursor

.import incsp1

.rodata

; Empty, Chest, Open, Key
.export METATILE0, METATILE1, METATILE2, METATILE3, METATILE4
METATILE0: .byte  $A0,  $00,  $8E,  $00,  $80,  $82,  $80,  $84,  $86,  $88,  $86,  $8A,  $86,  $88,  $86,  $8A
METATILE1: .byte  $A1,  $00,  $8F,  $00,  $81,  $83,  $81,  $85,  $87,  $89,  $87,  $8B,  $87,  $89,  $87,  $8B
METATILE2: .byte  $B0,  $00,  $9E,  $00,  $90,  $92,  $90,  $94,  $96,  $98,  $96,  $9A,  $96,  $98,  $96,  $9A
METATILE3: .byte  $B1,  $00,  $9F,  $00,  $91,  $93,  $91,  $95,  $97,  $99,  $97,  $9B,  $97,  $99,  $97,  $9B
METATILE4: .byte PAL2, PAL0, PAL2, PAL0, PAL0, PAL0, PAL1, PAL1, PAL0, PAL0, PAL1, PAL1, PAL0, PAL0, PAL1, PAL1

.code

.proc exec_set_metatile
	_addr = px_nmi_tmp + 0
	_attr = px_nmi_tmp + 2
	
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

.export _buffer_set_metatile
.proc _buffer_set_metatile
	_index = 0
	_addr = ptr1
	_qmask = tmp1
	cmd_bytes = (2 + 7)
	
	; Save tile address.
	sta _addr + 0
	stx _addr + 1
	
	; Write tile address.
	ldy px_buffer_cursor
	px_buffer_write_ax 1
	
	; Write tile index.
	ldx px_buffer_cursor
	ldy #(_index)
	lda (sp), y
	px_buffer_write_arg 0
	
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
	px_buffer_write_arg 3
	
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
	px_buffer_write_arg 4
	
	; Write quadrant mask.
	lda _qmask
	eor #$FF
	px_buffer_write_arg 5
	
	; Write attribute byte.
	ldy #(_index)
	lda (sp), y
	tay
	lda METATILE4, y
	and _qmask
	px_buffer_write_arg 6
	
	px_buffer_write_func exec_set_metatile
	
	lda px_buffer_cursor
	add #cmd_bytes
	sta px_buffer_cursor

	jmp incsp1
.endproc
