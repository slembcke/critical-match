.include "pixler.inc"

.importzp px_nmi_ready
.importzp px_ticks
.importzp px_mask, px_ctrl
.importzp PX_scroll_x
.importzp PX_scroll_y
.import _px_buffer_exec
.import FamiToneUpdate

.export px_nmi

.code

; http://forums.nesdev.com/viewtopic.php?f=2&t=16911
; Returns the quotient in x, remainder in a.
.macro div240_quick var
	; Start with q = n/256
	ldx var+1

	; r += 16*q
	txa
	asl
	asl
	asl
	asl
	; Overflows for > 12 bits!
	; Assume carry cleared by asl.

	adc var+0
	; Check for overflow, and adjust q and r again.
	bcc :+
		; q += 1
		inx
		; r += 16
		adc #15 ; +1 for carry flag
	:

	; Check if r > 240 and adjust once more.
	cmp #240
	bcc :+
		inx
		sbc #240
	:
.endmacro

.proc px_nmi
	; Interrupt enter.
	pha
	txa
	pha
	tya
	pha

	lda px_nmi_ready
	beq @skip_frame
	
	lda px_mask
	sta PPU_MASK
	
	lda px_ctrl
	sta PPU_CTRL
	
	; Invoke OAM DMA copy.
	lda #>OAM
	sta APU_SPR_DMA
	
	; Execute the display list buffer.
	jsr _px_buffer_exec
	
	; Reset PPU Address
	lda #0
	sta PPU_VRAM_ADDR
	sta PPU_VRAM_ADDR
	
	; Set the scroll registers.
	lda PX_scroll_x+0
	sta PPU_SCROLL
	div240_quick PX_scroll_y
	sta PPU_SCROLL
	; Note: Y-scroll remainder is in x.
	
	; Set nametable base address.
	lda PX_scroll_x+1
	lsr a
	txa
	rol a
	and #$03
	ora px_ctrl
	sta PPU_CTRL
	
	; Reset ready flag.
	lda #0
	sta px_nmi_ready
	
	@skip_frame:
	jsr FamiToneUpdate
	
	; Interrupt exit.
	pla
	tay
	pla
	tax
	pla
	
	rti
.endproc

.export _px_wait_nmi
.proc _px_wait_nmi
	lda #1
	sta px_nmi_ready
	:	lda px_nmi_ready
		bne :-
	
	inc px_ticks
	rts
.endproc
