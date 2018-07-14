; TODO seed this so it can be moved  out of ZP
.data

seed: .word $C4B6

.code

.export _lfsr8
.proc _lfsr8
	; iteration count
	ldx #8
	
	lda seed+0
	@loop:
		asl
		rol seed+1
		bcc :+
			; apply XOR feedback whenever a 1 bit is shifted out
			eor #$2D
		:
		dex
		bne @loop
	sta seed+0
	
	; reload flags
	cmp #0
	rts
.endproc
