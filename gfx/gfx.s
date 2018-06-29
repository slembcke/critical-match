.rodata

.export _neschar_inc
_neschar_inc:
	.include "neschar.inc"

.export _gfx_sheet1_lz4chr
_gfx_sheet1_lz4chr:
	.incbin "gfx/sheet1.lz4chr", 8

.export _gfx_squidman_lz4chr
_gfx_squidman_lz4chr:
	.incbin "gfx/squidman.lz4chr", 8
