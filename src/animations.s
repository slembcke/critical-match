BASE_CHR = $A0
FRAME_COUNT = 14
STRIDE = FRAME_COUNT*2

PAL = 2

.macro right_frame n
	.scope
		attr = $00
		chr = BASE_CHR + 2*n
		.byte  0, chr + 0 + 0*STRIDE, attr,  0
		.byte  0, chr + 1 + 0*STRIDE, attr,  8
		.byte  8, chr + 0 + 1*STRIDE, attr,  0
		.byte  8, chr + 1 + 1*STRIDE, attr,  8
		.byte 16, chr + 0 + 2*STRIDE, attr,  0
		.byte 16, chr + 1 + 2*STRIDE, attr,  8
	.endscope
.endmacro

.macro left_frame n
	.scope
		attr = $70
		chr = BASE_CHR + 2*n
		.byte  0, chr + 1 + 0*STRIDE, attr,  0
		.byte  0, chr + 0 + 0*STRIDE, attr,  8
		.byte  8, chr + 1 + 1*STRIDE, attr,  0
		.byte  8, chr + 0 + 1*STRIDE, attr,  8
		.byte 16, chr + 1 + 2*STRIDE, attr,  0
		.byte 16, chr + 0 + 2*STRIDE, attr,  8
	.endscope
.endmacro

.rodata

.export _animation
_animation:
	.repeat FRAME_COUNT, i
		right_frame i
	.endrepeat
