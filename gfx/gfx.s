.rodata

lz4_header_bytes = 8

.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, lz4_header_bytes
.endmacro

inclz4 _gfx_pixelakes_lz4chr, "pixelakes.lz4chr"
inclz4 _gfx_neschar_lz4chr, "neschar.lz4chr"
inclz4 _gfx_sheet1_lz4chr, "sheet1.lz4chr"
inclz4 _gfx_explosion_lz4chr, "explosion.lz4chr"
inclz4 _gfx_squidman_lz4chr, "squidman.lz4chr"
inclz4 _gfx_pixelakes_lz4, "pixelakes.lz4"
inclz4 _gfx_main_menu_lz4, "main_menu.lz4"
inclz4 _gfx_board_lz4, "board.lz4"
