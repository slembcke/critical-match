lz4_header_bytes = 8

.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, lz4_header_bytes
		.word 0 ; terminator
.endmacro

; .segment "PRG0"
.rodata

inclz4 _gfx_chr0_lz4chr, "CHR0.lz4chr"
inclz4 _gfx_explosion_lz4chr, "explosion.lz4chr"
inclz4 _gfx_character_lz4chr, "character.lz4chr"

inclz4 _gfx_menu_lz4, "menu.lz4"
inclz4 _gfx_credits_lz4, "credits.lz4"
inclz4 _gfx_board_lz4, "board.lz4"
inclz4 _gfx_game_over_lz4, "game_over.lz4"

.export _gfx_shapes
_gfx_shapes:
	.incbin "shapes.bin"
