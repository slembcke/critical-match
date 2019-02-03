.macpack generic

.include "zeropage.inc"
.import pushax, popax, _exit

.include "pixler.inc"
.include "pixler_lz4.inc"

.import _px_blit

.import px_lz4_read_src
.import px_lz4_src_to_dst
.import px_lz4_dst_to_dst
.import px_lz4

.code

.proc ram_to_vram
	; Set the VRAM address.
	lda	dst+1
	sta	PPU_VRAM_ADDR
	lda	dst+0
	sta	PPU_VRAM_ADDR
	
	; ; Currently doesn't handle literal runs of > 255
	; lda offset+1
	; beq :+
	; 	jmp _exit
	; :
	
	ldx offset+0
	@loop:
		beq @loop_end
		
		jsr px_lz4_read_src
		sta PPU_VRAM_IO
		
		inc dst+0
		bne :+
			inc dst+1
		:
		
		dex
		bne @loop
	@loop_end:
	
	rts
.endproc

; dst: ptr2, src: ptr1, len: ptr3
.proc vram_to_vram
	lda offset+0
	; eor #$FF
	; iny
	tay
	
	@loop:
		; lda	offset+0
		tya
		ora	offset+1
		bne	:+
			rts
		:
		
		; Set source address.
		lda	ptr1+1
		sta	PPU_VRAM_ADDR
		lda	ptr1+0
		sta	PPU_VRAM_ADDR
		
		; Read twice, the first value is garbage.
		ldx	PPU_VRAM_IO
		ldx	PPU_VRAM_IO
		
		; Set destination address.
		lda	dst+1
		sta	PPU_VRAM_ADDR
		lda	dst+0
		sta	PPU_VRAM_ADDR
		
		; Write byte.
		stx	PPU_VRAM_IO
		
		; Update counters.
		inc	ptr1+0
		bne	:+
			inc	ptr1+1
		:
		
		inc	dst+0
		bne	:+
			inc	dst+1
		:
		
		; dey
		; bne :+
		; 	dec offset+1
		; :
		
		; increase counter
		tya
		sub #1
		tay
		lda offset+1
		sbc #0
		sta offset+1
		
		jmp @loop
.endproc

.export _px_lz4_to_vram
.proc _px_lz4_to_vram
	jsr pushax
	
	lda #<ram_to_vram
	ldx #>ram_to_vram
	sta px_lz4_src_to_dst+1
	stx px_lz4_src_to_dst+2
	
	lda #<vram_to_vram
	ldx #>vram_to_vram
	sta px_lz4_dst_to_dst+1
	stx px_lz4_dst_to_dst+2
	
	jmp px_lz4
.endproc
