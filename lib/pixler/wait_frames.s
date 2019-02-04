.import _px_wait_nmi

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
