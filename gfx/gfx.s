lz4_header_bytes = 8

.macro inclz4 symbol, file
	.export symbol
	symbol:
		.incbin file, lz4_header_bytes
		.word 0 ; terminator
.endmacro

.segment "PRG0"

inclz4 _gfx_pixelakes_lz4chr, "pixelakes.lz4chr"
inclz4 _gfx_logo64_lz4chr, "logo64.lz4chr"
inclz4 _gfx_menu_tiles_lz4chr, "menu_tiles.lz4chr"
inclz4 _gfx_neschar_lz4chr, "neschar.lz4chr"
inclz4 _gfx_sheet1_lz4chr, "sheet1.lz4chr"
inclz4 _gfx_explosion_lz4chr, "explosion.lz4chr"

inclz4 _gfx_squidman_lz4chr, "characters/squidman.lz4chr"
inclz4 _gfx_azmodeus_lz4chr, "characters/azmodeus.lz4chr"
inclz4 _gfx_pinkblob_lz4chr, "characters/pinkblob.lz4chr"
inclz4 _gfx_blob_lz4chr, "characters/blob.lz4chr"
inclz4 _gfx_budgie_lz4chr, "characters/budgie.lz4chr"
inclz4 _gfx_robinhood_lz4chr, "characters/robinhood.lz4chr"
inclz4 _gfx_bonecrusher_lz4chr, "characters/bonecrusher.lz4chr"
inclz4 _gfx_dagon_lz4chr, "characters/dagon.lz4chr"
inclz4 _gfx_dog_lz4chr, "characters/dog.lz4chr"
inclz4 _gfx_eyeboll_lz4chr, "characters/eyeboll.lz4chr"
inclz4 _gfx_kermit_lz4chr, "characters/kermit.lz4chr"
inclz4 _gfx_ninja_lz4chr, "characters/ninja.lz4chr"
inclz4 _gfx_orc_lz4chr, "characters/orc.lz4chr"
inclz4 _gfx_royalguard_lz4chr, "characters/royalguard.lz4chr"
inclz4 _gfx_rustknight_lz4chr, "characters/rustknight.lz4chr"
inclz4 _gfx_cyclops_lz4chr, "characters/cyclops.lz4chr"

inclz4 _gfx_pixelakes_lz4, "pixelakes.lz4"
inclz4 _gfx_main_menu_lz4, "main_menu.lz4"
inclz4 _gfx_character_select_lz4, "character_select.lz4"
inclz4 _gfx_board_lz4, "board.lz4"
inclz4 _gfx_game_over_lz4, "game_over.lz4"
