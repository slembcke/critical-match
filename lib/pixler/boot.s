.include "zeropage.inc"
.include "pixler.inc"

.import NES_MAPPER, NES_PRG_BANKS, NES_CHR_BANKS, NES_MIRRORING
.import __CSTACK_START__, __CSTACK_SIZE__
.import copydata
.import _waitvsync
.import _main
.importzp px_ctrl
.import px_nmi

.export start, _exit = start
.export __STARTUP__:absolute = 1
.export _OAM = OAM

.segment "HEADER"

	.byte "NES", $1a ; iNES Magic number.
	.byte <NES_PRG_BANKS
	.byte <NES_CHR_BANKS
	.byte <((<NES_MAPPER<<4) | NES_MIRRORING)
	.byte <(NES_MAPPER & $F0)
	.res 8, 0 ; Remainder of header is unused space.

.segment "STARTUP"

.proc start
	sei ; Disable interrupts.
	cld ; Disable decimal mode.
	
	; Disable interrupts for the APU.
	lda #APU_FRAME_COUNTER_IRQ_INHIBIT
	sta APU_FRAME_COUNTER
	
	; Disable/reset the PPU
	lda #0
	sta PPU_CTRL
	sta PPU_MASK
	
	; Disable the DMC.
	sta APU_MODCTRL
	
	; Set the stack pointer.
	ldx #$FF
	txs
	
	; Wait for the first of two vblanks.
	jsr _waitvsync

	; Zero the RAM.
	lda #0
	tax ; Byte counter.
	:	sta $00, x
		sta $0100, x
		sta $0200, x
		sta $0300, x
		sta $0400, x
		sta $0500, x
		sta $0600, x
		sta $0700, x
		inx
		bne :-
	
	; Wait for the second of two vblanks.
	; After this PPU should definitely be warmed up and ready to use!
	jsr _waitvsync
	
	; Clear PPU memory.
	; This includes a lot of duplicate writes to
	; mirrored memory, but it doesn't really matter.
	lda #0
	sta PPU_VRAM_ADDR
	sta PPU_VRAM_ADDR
	ldy #$40 ; Number of pages.
	ldx #0 ; Byte counter.
	:	sta PPU_VRAM_IO
		inx
		bne :-
		dey
		bne :-
	
	; Set BG color to black.
	lda #>PPU_PAL0
	sta PPU_VRAM_ADDR
	lda #<PPU_PAL0
	sta PPU_VRAM_ADDR
	lda #$30
	sta PPU_VRAM_IO
	
	; Move sprites offscreen.
	ldy #64 ; Sprite counter.
	ldx #0 ; Byte counter.
	lda #240 ; y positions past 240 are offscreen.
	:	sta OAM, x ; Store y position;
		; Skip 4 bytes to the next sprite.
		inx
		inx
		inx
		inx
		dey
		bne :-
	
	; Reset sprite address register.
	lda #0
	sta PPU_SPR_ADDR
	
	; DMA copy sprites to the PPU.
	lda #>OAM
	sta APU_SPR_DMA
	
	; Enable the PPU NMI.
	lda #$80
	sta px_ctrl
	sta PPU_CTRL
	
	; Initialize the C runtime and jump to main().
	jsr copydata
	
	CSTACK_TOP = __CSTACK_START__ + __CSTACK_SIZE__
	lda #<CSTACK_TOP
	sta sp + 0
	lda #>CSTACK_TOP
	sta sp + 1
	
	jmp _main
.endproc

.code

.proc irq
	rti
.endproc

.segment "VECTORS"
	.word px_nmi
	.word start
	.word irq
