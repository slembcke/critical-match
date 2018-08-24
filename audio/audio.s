.import FamiToneInit
.import FamiToneUpdate
.import FamiToneMusicPlay
.import FamiToneMusicPause
.import FamiToneMusicStop

; .code

.export _music_init
.proc _music_init ; u16 addr
	; ax -> xy
	pha
	txa
	tay
	pla
	tax
	lda #1 ; TODO Hardcoded NTSC
	jsr FamiToneInit
	
	rts
.endproc

.export _music_play = FamiToneMusicPlay
.export _music_pause = FamiToneMusicPause
.export _music_stop = FamiToneMusicStop

.rodata

.export _TITLE_MUSIC
_TITLE_MUSIC:
	.include "title2.s"

.export _CHARACTER_SELECT_MUSIC
_CHARACTER_SELECT_MUSIC:
	.include "character-select.s"

.export _GAMEPLAY_MUSIC
_GAMEPLAY_MUSIC:
	.include "gameplay2.s"
