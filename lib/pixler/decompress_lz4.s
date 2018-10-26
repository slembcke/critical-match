;
; Lauri Kasanen, 6 Jun 2017
; (C) Mega Cat Studios
; An optimized LZ4 decompressor
;

; Modified by slembcke to support decompressing to either RAM or VRAM.

.include "zeropage.inc"
.include "pixler.inc"

.macpack longbranch
.import pushax, popax
.import	memcpy_upwards
.import _px_blit

out = regsave
written = regsave + 2
tmp = tmp1
token = tmp2
offset = ptr3
in = sreg

.data

; Jump vectors for the memcpy functions.
memcpy_src_to_dst: jmp $FFFC
memcpy_dst_to_dst: jmp $FFFC

.code

; These memcp-like functions are drop in replacements for memcpy_upwards().
; They are patched in using the jump vectors above when decompressing to vram.

.proc memcpy_ram_to_vram
	lda	ptr2+1
	sta	PPU_VRAM_ADDR
	lda	ptr2+0
	sta	PPU_VRAM_ADDR
	
	lda	ptr3+0
	ldx	ptr3+1
	jsr	pushax
	
	lda	ptr1+0
	ldx	ptr1+1
	
	jsr	_px_blit
	jmp popax
.endproc

.proc memcpy_vram_to_vram
	lda	#0
	sta	tmp3
	sta	tmp4
	jmp	@check
	
	@loop:
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

		@check:
		lda	tmp3
		cmp	ptr3+0
		lda	tmp4
		sbc	ptr3+1
		bcc	@loop
	jmp popax
.endproc

.export _decompress_lz4_to_ram
.proc _decompress_lz4_to_ram
	jsr pushax
	
	lda #<memcpy_upwards
	ldx #>memcpy_upwards
	sta memcpy_src_to_dst+1
	stx memcpy_src_to_dst+2
	sta memcpy_dst_to_dst+1
	stx memcpy_dst_to_dst+2
	
	jmp decompress_lz4
.endproc

.export _decompress_lz4_to_vram
.proc _decompress_lz4_to_vram
	jsr pushax
	
	lda #<memcpy_ram_to_vram
	ldx #>memcpy_ram_to_vram
	sta memcpy_src_to_dst+1
	stx memcpy_src_to_dst+2
	
	lda #<memcpy_vram_to_vram
	ldx #>memcpy_vram_to_vram
	sta memcpy_dst_to_dst+1
	stx memcpy_dst_to_dst+2
	
	jmp decompress_lz4
.endproc

.proc	decompress_lz4
	jsr	popax
	sta	in+0
	stx	in+1
	
	jsr	popax
	sta	out+0
	stx	out+1
	
	; written = 0;
	lda #0
	sta written+0
	sta written+1
	
	loop:
	; token = *in++;
	ldy #0
	lda (in), y
	sta token

	inc in
	bne :+
		inc in+1
	:
	
	; offset = token >> 4;
	ldx #0
	lsr a
	lsr a
	lsr a
	lsr a
	sta offset+0
	stx offset+1
	
	; token &= 0xf;
	; token += 4; // Minmatch
	lda token
	and #$0F
	clc
	adc #4
	sta token
	
	; if (offset == 15) {
	lda offset+0
	cmp #15
	moreliterals:
	bne L001A
	
	; tmp = *in++;
	ldy #0
	lda (in), y
	sta tmp
	
	inc in+0
	bne :+
		inc in+1
	:
	
	; offset += tmp;
	clc
	adc offset+0
	sta offset+0
	lda #0
	adc offset+1
	sta offset+1
	
	; if (tmp == 255)
	lda tmp
	cmp #255
	
	; goto moreliterals;
	jmp moreliterals
	
	L001A:
	; if (offset) {
	lda offset+0
	ora offset+1
	beq L001C
	
	; memcpy(&out[written], in, offset);
	lda out+0
	clc
	adc written+0
	sta ptr2+0
	lda out+1
	adc written+1
	tax
	lda ptr2+0
	stx ptr2+1
	jsr pushax
	lda in+0
	ldx in+1
	sta ptr1+0
	stx ptr1+1
	; ldy #$00 - not needed as pushax zeroes Y
	jsr memcpy_src_to_dst
	
; written += offset;
	lda offset+0
	clc
	adc written+0
	sta written+0
	lda offset+1
	adc written+1
	sta written+1
	
; in += offset;
	lda offset+0
	clc
	adc in+0
	sta in+0
	lda offset+1
	adc in+1
	sta in+1
	L001C:
	
	; memcpy(&offset, in, 2);
	ldy #0
	lda (in), y
	sta offset
	iny
	lda (in), y
	sta offset+1
	
	; Terminate if offset is 0.
	lda offset+0
	bne :+
		lda offset+1
		bne :+
		rts
	:
	
	; in += 2;
	lda #2
	clc
	adc in+0
	sta in+0
	bcc :+
		inc in+1
	:
	
	; copysrc = out + written - offset;
	lda out+0
	clc
	adc written+0
	tay
	lda out+1
	adc written+1
	tax
	tya
	sec
	sbc offset+0
	sta ptr1+0
	txa
	sbc offset+1
	sta ptr1+1
	
	; offset = token;
	lda #0
	sta offset+1
	lda token
	sta offset+0
	
	; if (token == 19) {
	cmp #19
	morematches:
	bne L003C
	
	; tmp = *in++;
	ldy #0
	lda (in), y
	sta tmp
	
	inc in+0
	bne :+
		inc in+1
	:
	
	; offset += tmp;
	clc
	adc offset+0
	sta offset+0
	tya
	adc offset+1
	sta offset+1
	
	; if (tmp == 255)
	lda tmp
	cmp #255
	
	; goto morematches;
	jmp morematches
	
	L003C:
	; memcpy(&out[written], copysrc, offset);
	lda out
	clc
	adc written+0
	sta ptr2+0
	lda out+1
	adc written+1
	tax
	lda ptr2+0
	stx ptr2+1
	jsr pushax
	; ldy #$00 - not needed as pushax zeroes Y
	jsr memcpy_dst_to_dst
	
	; written += offset;
	lda offset+0
	clc
	adc written+0
	sta written+0
	lda offset+1
	adc written+1
	sta written+1
	
	jmp loop
.endproc
