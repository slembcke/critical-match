.include "pixler.inc"

.importzp px_ticks
.importzp PX_scroll_x
.importzp PX_scroll_y
.import _px_buffer_exec
.import FamiToneUpdate

.export px_nmi

.zeropage

px_nmi_ready: .byte 0

.code

.proc px_nmi
	; Interrupt enter.
	pha
	txa
	pha
	tya
	pha

	lda px_nmi_ready
	beq @skip_frame
	
	; Reset ready flag.
	lda #0
	sta px_nmi_ready
	
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
	lda PX_scroll_x + 0
	sta PPU_SCROLL
	lda PX_scroll_y + 0
	sta PPU_SCROLL
	
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

.export _px_wait_frames
.proc _px_wait_frames ; u8 frames
	tax
	:	cpx #0
		beq :+
		jsr _px_wait_nmi
		dex
		jmp :-
	:
	
	rts
.endproc
