;
; Lauri Kasanen, 6 Jun 2017
; (C) Mega Cat Studios
; An optimized LZ4 decompressor
;

; Modified by slembcke to support decompressing to either RAM or VRAM.

.macpack generic
.include "zeropage.inc"
.import pushax, popax

.include "pixler.inc"
.include "pixler_lz4.inc"

.data

.export px_lz4_src_to_dst, px_lz4_dst_to_dst
; Jump vectors for the memcpy functions.
px_lz4_src_to_dst: jmp $FFFC
px_lz4_dst_to_dst: jmp $FFFC

.code

.export px_lz4_read_src
.proc px_lz4_read_src
	ldy #0
	lda (src), y

	inc src+0
	bne :+
		inc src+1
	:
	
	rts
.endproc

.export px_lz4
.proc	px_lz4
	jsr	popax
	sta	src+0
	stx	src+1
	
	jsr	popax
	sta	dst+0
	stx	dst+1
	
	@loop:
	; get_token
	jsr px_lz4_read_src
	sta token
	
	; Decode literal count from token upper nibble.
	lsr a
	lsr a
	lsr a
	lsr a
	sta offset+0
	ldx #0
	stx offset+1
	
	; if (offset == 15) {
	cmp #15
	jsr consume_length_bytes
	
	; Copy literals
	jsr px_lz4_src_to_dst
	
	; memcpy(&offset, src, 2);
	jsr px_lz4_read_src
	sta offset
	jsr px_lz4_read_src
	sta offset+1
	
	; Terminate if offset is 0.
	lda offset+0
	ora offset+1
	bne :+
		rts
	:
	
	; copysrc = dst - offset;
	lda dst+0
	sub offset+0
	sta ptr1+0
	lda dst+1
	sbc offset+1
	sta ptr1+1
	
	lda token
	and #$0F
	add #4
	sta offset+0
	ldx #0
	stx offset+1
	
	; if (token == 19) {
	cmp #19
	jsr consume_length_bytes
	
	; memcpy(dst, copysrc, offset);
	jsr px_lz4_dst_to_dst
	
	jmp @loop
.endproc

.proc consume_length_bytes
	beq :+
		rts
	:
	
	; tmp = *src++;
	ldy #0
	lda (src), y
	tax
	
	inc src+0
	bne :+
		inc src+1
	:
	
	; offset += tmp;
	add offset+0
	sta offset+0
	bcc :+
		inc offset+1
	:
	
	; if (tmp == 255)
	txa
	cmp #255
	
	jmp consume_length_bytes
.endproc
