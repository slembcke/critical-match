.import FamiToneInit
.import FamiToneSfxInit
.import FamiToneUpdate
.import FamiToneMusicPlay
.import FamiToneMusicPause
.import FamiToneMusicStop
.import FamiToneSfxPlay

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

.export _sound_init
.proc _sound_init ; u16 addr
	; ax -> xy
	pha
	txa
	tay
	pla
	tax
	jsr FamiToneSfxInit
	
	rts
.endproc

.export _music_play = FamiToneMusicPlay
.export _music_pause = FamiToneMusicPause
.export _music_stop = FamiToneMusicStop
.export _sound_play = FamiToneSfxPlay

.rodata

.export _MUSIC
_MUSIC:
	.include "gameplay2.s"

.export _SOUNDS
_SOUNDS:
.include "sounds.s"