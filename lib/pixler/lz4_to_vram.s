.macpack generic

.include "zeropage.inc"
.import pushax, popax, _exit

.include "pixler.inc"
.include "lz4.inc"

.import _px_blit

.import px_lz4_read_src
.import px_lz4_src_to_dst
.import px_lz4_dst_to_dst
.import px_lz4

.code

.proc ram_to_vram
	; Set the VRAM address.
	ldx	dst+1
	stx	PPU_VRAM_ADDR
	lda	dst+0
	sta	PPU_VRAM_ADDR
	
	; add run_length+0
	; sta dst+0
	; txa
	; adc run_length+1
	; sta dst+1
	
	ldx run_length+0
	@loop:
		txa
		ora run_length+1
		beq @loop_end
		
		jsr px_lz4_read_src
		sta PPU_VRAM_IO
		
		inc dst+0
		bne :+
			inc dst+1
		:
		
		; Decrement counter.
		txa
		bne :+
			dec run_length+1
		:
		dex
		jmp @loop
	@loop_end:
	
	rts
.endproc

.proc vram_to_vram
	ldx run_length+0
	
	@loop:
		txa
		ora	run_length+1
		bne	:+
			rts
		:
		
		; Set source address.
		lda	back_src+1
		sta	PPU_VRAM_ADDR
		lda	back_src+0
		sta	PPU_VRAM_ADDR
		
		; Read twice, the first value is garbage.
		ldy	PPU_VRAM_IO
		ldy	PPU_VRAM_IO
		
		; Set destination address.
		lda	dst+1
		sta	PPU_VRAM_ADDR
		lda	dst+0
		sta	PPU_VRAM_ADDR
		
		; Write byte.
		sty	PPU_VRAM_IO
		
		; Increment pointers.
		inc	back_src+0
		bne	:+
			inc	back_src+1
		:
		
		inc	dst+0
		bne	:+
			inc	dst+1
		:
		
		; Decrement counter.
		txa
		bne :+
			dec run_length+1
		:
		dex
		
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
