.include "nes.inc"

.import __OAM_START__
OAM = __OAM_START__
OAM_X = __OAM_START__ + 3
OAM_Y = __OAM_START__ + 0
OAM_CHR = __OAM_START__ + 1
OAM_ATTR = __OAM_START__ + 2

; Some more descriptive aliases for the cc65 defines.
APU_FRAME_COUNTER = APU_PAD2
APU_FRAME_COUNTER_IRQ_INHIBIT = $40

PPU_CTRL = PPU_CTRL1
PPU_MASK = PPU_CTRL2
PPU_SCROLL = PPU_VRAM_ADDR1
PPU_VRAM_ADDR = PPU_VRAM_ADDR2

PPU_NAMETBL0 = $2000
PPU_PAL0 = $3F00

PAL0 = %00000000
PAL1 = %01010101
PAL2 = %10101010
PAL3 = %11111111

QUADRANT_MASK: .byte %00000011, %00001100, %00110000, %11000000

.macro px_buffer_write_func f
	lda #<(f - 1)
	sta $0100, x
	lda #>(f - 1)
	sta $0101, x
.endmacro

.macro px_buffer_write_arg idx, value
	.scope
		.ifnblank value
			lda #value
		.endif
		
		addr = $0102 + idx
		sta addr, x
	.endscope
.endmacro

.macro px_buffer_write_ax idx
	.scope
		sta $0103 + idx, y
		txa
		sta $0102 + idx, y
	.endscope
.endmacro
