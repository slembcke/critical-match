.import pushax
.import memcpy_upwards

.import px_lz4
.import px_lz4_src_to_dst
.import px_lz4_dst_to_dst

.code

.export _px_lz4_to_ram
.proc _px_lz4_to_ram
	jsr pushax
	
	lda #<memcpy_upwards
	ldx #>memcpy_upwards
	sta px_lz4_src_to_dst+1
	stx px_lz4_src_to_dst+2
	sta px_lz4_dst_to_dst+1
	stx px_lz4_dst_to_dst+2
	
	jmp px_lz4
.endproc
