.import FamiToneInit
.import FamiToneUpdate
.import FamiToneMusicPlay
.import FamiToneMusicPause
.import FamiToneMusicStop

; .code

.export _music_init
.proc _music_init ; u16 addr
	lda #1 ; TODO Hardcoded NTSC
	ldx #<_MUSIC
	ldy #>_MUSIC
	jsr FamiToneInit
	
	lda #0
	jsr FamiToneMusicPlay
	
	rts
.endproc

.export _music_play = FamiToneMusicPlay
.export _music_pause = FamiToneMusicPause
.export _music_stop = FamiToneMusicStop

.rodata

.export _MUSIC
_MUSIC:
	.include "famitone2/space_radar_menu.inc"
