.include "nes.inc"

; Config
OAM = $0200

; Some more descriptive aliases for the cc65 defines.
APU_FRAME_COUNTER = APU_PAD2
APU_FRAME_COUNTER_IRQ_INHIBIT = $40

PPU_CTRL = PPU_CTRL1
PPU_MASK = PPU_CTRL2
PPU_SCROLL = PPU_VRAM_ADDR1
PPU_VRAM_ADDR = PPU_VRAM_ADDR2

PPU_NAMETBL0 = $2000
PPU_PAL0 = $3F00

.macro c_var idx
	ldy #(idx)
	lda (sp), y
.endmacro

.macro interrupt_enter
	pha
	txa
	pha
	tya
	pha
.endmacro

.macro interrupt_exit
	pla
	tay
	pla
	tax
	pla
	
	rti
.endmacro
