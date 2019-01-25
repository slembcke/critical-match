lz4_header_bytes = 8

.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, lz4_header_bytes
		.word 0 ; terminator
.endmacro

; .segment "PRG0"
.rodata

inclz4 _gfx_CHR0_lz4chr, "CHR0.lz4chr"

inclz4 _gfx_splash_lz4, "splash.lz4"
