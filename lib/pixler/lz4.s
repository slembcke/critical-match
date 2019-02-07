; Originally based on some lz4 code by Lauri Kasanen and Shiru.
; I went through and rewrote the whole thing for size,
; and I don't think there is much left of the original code.

.macpack generic
.include "zeropage.inc"
.import pushax, popax

.include "pixler.inc"
.include "lz4.inc"

.data

; Jump vectors for the memcpy functions.
.export px_lz4_src_to_dst
px_lz4_src_to_dst: jmp $FFFC

.export px_lz4_dst_to_dst
px_lz4_dst_to_dst: jmp $FFFC

.code

; Read a byte from the input stream.
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

; Decompress lz4 data (w/o the header)
; Requires px_lz4_src_to_dst and px_lz4_dst_to_dst to be set.
.export px_lz4
.proc	px_lz4
	jsr	popax
	sta	src+0
	stx	src+1
	
	jsr	popax
	sta	dst+0
	stx	dst+1
	
	@loop:
	; Read and save token
	jsr px_lz4_read_src
	pha
	
	; Decode literal count from token upper nibble.
	lsr a
	lsr a
	lsr a
	lsr a
	sta run_length+0
	ldx #0
	stx run_length+1
	
	; Check if we need to consume more length bytes
	cmp #15
	jsr consume_length_bytes
	
	; Copy literals
	jsr px_lz4_src_to_dst
	
	; Read offset value.
	jsr px_lz4_read_src
	sta offset+0
	jsr px_lz4_read_src
	sta offset+1
	
	; Terminate if offset is 0.
	ora offset+0
	bne :+
		pla
		rts
	:
	
	; Calculate backref start.
	lda dst+0
	sub offset+0
	sta back_src+0
	lda dst+1
	sbc offset+1
	sta back_src+1
	
	; Decode backref length from token lower nibble.
	pla
	and #$0F
	add #4
	sta run_length+0
	; ldx #0 ; x is zeroed by px_lz4_src_to_dst
	stx run_length+1
	
	; Check if we have to consume additional length bytes.
	cmp #19
	jsr consume_length_bytes
	
	; Copy backref bytes.
	jsr px_lz4_dst_to_dst
	
	jmp @loop
.endproc

.proc consume_length_bytes
	beq :+
		rts
	:
	
	jsr px_lz4_read_src
	tax
	
	add run_length+0
	sta run_length+0
	bcc :+
		inc run_length+1
	:
	
	; Max value means we need to consume more run length bytes.
	txa
	cmp #$FF
	
	jmp consume_length_bytes
.endproc
