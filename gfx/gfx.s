.rodata

lz4_header_bytes = 8

.export _gfx_pixelakes_lz4chr
_gfx_pixelakes_lz4chr:
	.incbin "pixelakes.lz4chr", lz4_header_bytes

.export _gfx_neschar_lz4chr
_gfx_neschar_lz4chr:
	.incbin "neschar.lz4chr", lz4_header_bytes

.export _gfx_sheet1_lz4chr
_gfx_sheet1_lz4chr:
	.incbin "sheet1.lz4chr", lz4_header_bytes

.export _gfx_squidman_lz4chr
_gfx_squidman_lz4chr:
	.incbin "squidman.lz4chr", lz4_header_bytes

.export _gfx_pixelakes_lz4
_gfx_pixelakes_lz4:
	.incbin "pixelakes.lz4", lz4_header_bytes

.export _gfx_main_menu_lz4
_gfx_main_menu_lz4:
	.incbin "main_menu.lz4", lz4_header_bytes

.export _gfx_board_lz4
_gfx_board_lz4:
	.incbin "board.lz4", lz4_header_bytes
