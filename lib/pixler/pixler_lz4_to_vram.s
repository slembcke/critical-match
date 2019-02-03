.macpack generic

.include "zeropage.inc"
.import pushax, popax

.include "pixler.inc"
.include "pixler_lz4.inc"

.import _px_blit

.import px_lz4
.import px_lz4_src_to_dst
.import px_lz4_dst_to_dst

.code

; These memcpy-like functions are drop in replacements for memcpy_upwards().
; They are patched in using the jump vectors above when decompressing to vram.

; dst: ptr2, src: ptr1, len: ptr3
.proc ram_to_vram
	lda	dst+1
	sta	PPU_VRAM_ADDR
	lda	dst+0
	sta	PPU_VRAM_ADDR
	
	lda	offset+0
	ldx	offset+1
	jsr	pushax
	lda	src+0
	ldx	src+1
	jsr	_px_blit
	
; dst += offset;
	lda dst+0
	add offset+0
	sta dst+0
	lda dst+1
	adc offset+1
	sta dst+1
	
; src += offset;
	lda src+0
	add offset+0
	sta src+0
	lda src+1
	adc offset+1
	sta src+1
	
	rts
.endproc

; dst: ptr2, src: ptr1, len: ptr3
.proc vram_to_vram
	lda	#0
	sta	tmp3
	sta	tmp4
	
	@loop:
		lda	tmp3
		cmp	ptr3+0
		lda	tmp4
		sbc	ptr3+1
		bcc	:+
			jmp popax
		:
		
		; read source byte
		lda	ptr1+1
		sta	PPU_VRAM_ADDR
		lda	ptr1+0
		sta	PPU_VRAM_ADDR
		
		ldx	PPU_VRAM_IO
		ldx	PPU_VRAM_IO
		
		inc	ptr1+0
		bne	:+
			inc	ptr1+1
		:
		
		; write dst byte
		lda	ptr2+1
		sta	PPU_VRAM_ADDR
		lda	ptr2+0
		sta	PPU_VRAM_ADDR
		
		stx	PPU_VRAM_IO
		
		inc	ptr2+0
		bne	:+
			inc	ptr2+1
		:
		
		; increase counter
		inc	tmp3
		bne	:+
			inc	tmp4
		:
		
		jmp @loop
.endproc

.export _decompress_lz4_to_vram
.proc _decompress_lz4_to_vram
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
