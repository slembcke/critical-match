.importzp px_buffer_cursor

.macro px_buffer_write_func f
	lda #<(f - 1)
	sta $0100, y
	lda #>(f - 1)
	sta $0101, y
.endmacro

.macro px_buffer_write_arg idx, value
	.scope
		.ifnblank value
			lda #value
		.endif
		
		addr = $0102 + idx
		sta addr, y
	.endscope
.endmacro

.macro px_buffer_write_ax idx
	.scope
		sta $0103 + idx, y
		txa
		sta $0102 + idx, y
	.endscope
.endmacro
