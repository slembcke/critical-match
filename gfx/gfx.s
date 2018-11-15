lz4_header_bytes = 8

.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, lz4_header_bytes
		.word 0 ; terminator
.endmacro

; .segment "PRG0"
.rodata

inclz4 _gfx_neschar_lz4chr, "neschar.lz4chr"
inclz4 _gfx_sheet1_lz4chr, "sheet1.lz4chr"
inclz4 _gfx_explosion_lz4chr, "explosion.lz4chr"

inclz4 _gfx_character_lz4chr, "character.lz4chr"

inclz4 _gfx_board_lz4, "board.lz4"
inclz4 _gfx_game_over_lz4, "game_over.lz4"
