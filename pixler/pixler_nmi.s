.include "pixler.inc"

.importzp px_ticks
.importzp PX_scroll_x
.importzp PX_scroll_y
.import px_buffer_exec

.export px_nmi
.export _px_wait_nmi = px_wait_nmi

.zeropage

px_nmi_ready: .byte 0

.code

.proc px_nmi
	interrupt_enter
	lda px_nmi_ready
	beq @skip_frame
	
	; Reset ready flag.
	lda #0
	sta px_nmi_ready
	
	; Invoke OAM DMA copy.
	lda #>OAM
	sta APU_SPR_DMA
	
	; Execute the display list buffer.
	jsr px_buffer_exec
	
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
	interrupt_exit
	rti
.endproc

.proc px_wait_nmi
	lda #1
	sta px_nmi_ready
	:	lda px_nmi_ready
		bne :-
	
	inc px_ticks
	rts
.endproc
