; https://wiki.nesdev.com/w/index.php/Random_number_generator

.zeropage

.export _rand_seed
_rand_seed: .res 2

.code

.export _rand8
.proc _rand8
	; iteration count
	ldx #8
	
	lda _rand_seed+0
	@loop:
		asl
		rol _rand_seed+1
		bcc :+
			; apply XOR feedback whenever a 1 bit is shifted out
			eor #$2D
		:
		dex
		bne @loop
	sta _rand_seed+0
	
	; reload flags
	cmp #0
	rts
.endproc
