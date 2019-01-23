lz4_header_bytes = 8

.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, lz4_header_bytes
		.word 0 ; terminator
.endmacro

; .segment "PRG0"
.rodata

inclz4 _gfx_menu_lz4, "menu.lz4"
inclz4 _gfx_credits_lz4, "credits.lz4"
inclz4 _gfx_board_lz4, "board.lz4"
inclz4 _gfx_game_over_lz4, "game_over.lz4"

.export _gfx_shapes
_gfx_shapes:
	.incbin "shapes.bin"

.segment "CHR"

.incbin "neschar.chr", 0, $800
.incbin "sheet1.chr", 0, $800

.incbin "neschar.chr", 0, $200
.incbin "explosion.chr", 0, $200
.res $400
.incbin "sheet1.chr", 0, $200
.incbin "character.chr"
