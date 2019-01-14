.macpack generic

; Fixed version of bgt macro that works with local labels.
.macro _bgt label
	beq :+
		bcs label
	:
.endmacro


.importzp _ix, _iy, _idx
.import _GRID, _COLUMN_HEIGHT

BLOCK_STATUS_UNLOCKED_AND_OPEN = $28

.rodata

.define MATCH_TABLE match_square - 1, match_S - 1, match_Z - 1, match_T - 1, match_L - 1, match_J - 1
MATCH_TABLE_L: .lobytes MATCH_TABLE
MATCH_TABLE_H: .hibytes MATCH_TABLE

.code

; shape: a
; x: _ix, y: _iy, block index: _idx
.export _grid_match_tetromino
.proc _grid_match_tetromino
	tax
	lda MATCH_TABLE_H, x
	pha
	lda MATCH_TABLE_L, x
	pha
	rts
.endproc

.proc match_square
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+8+0, y
	bne @no_match
	cmp _GRID+8+1, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	bge @no_match
	cmp _COLUMN_HEIGHT+1, x
	bge @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+8+0, y
	sta _GRID+8+1, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_S
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+8+1, y
	bne @no_match
	cmp _GRID+8+2, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	beq :+
		bge @no_match
	:
	cmp _COLUMN_HEIGHT+1, x
	bge @no_match
	cmp _COLUMN_HEIGHT+2, x
	bge @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+8+1, y
	sta _GRID+8+2, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_Z
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+8+0, y
	bne @no_match
	cmp _GRID+8-1, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT-1, x
	bge @no_match
	cmp _COLUMN_HEIGHT+0, x
	bge @no_match
	cmp _COLUMN_HEIGHT+1, x
	_bgt @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+8+0, y
	sta _GRID+8-1, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_T
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+0+2, y
	bne @no_match
	cmp _GRID+8+1, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+1, x
	bge @no_match
	cmp _COLUMN_HEIGHT+2, x
	_bgt @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+0+2, y
	sta _GRID+8+1, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_L
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+0+2, y
	bne @no_match
	cmp _GRID+8+2, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+1, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+2, x
	bge @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+0+2, y
	sta _GRID+8+2, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_rotJ
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+0+2, y
	bne @no_match
	cmp _GRID-8+2, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+1, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+2, x
	_bgt @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+0+2, y
	sta _GRID-8+2, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_J
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+0+2, y
	bne @no_match
	cmp _GRID+8+0, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	bge @no_match
	cmp _COLUMN_HEIGHT+1, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+2, x
	_bgt @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+0+2, y
	sta _GRID+8+0, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_rotL
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+0+2, y
	bne @no_match
	cmp _GRID-8+0, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+1, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+2, x
	_bgt @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+0+2, y
	sta _GRID-8+0, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

.proc match_rotT
	ldy _idx
	ldx _ix
	
	; Check blocks match.
	lda _GRID, y
	cmp _GRID+0+1, y
	bne @no_match
	cmp _GRID+0+2, y
	bne @no_match
	cmp _GRID-8+1, y
	bne @no_match
	
	; Check rows exist.
	lda _iy
	cmp _COLUMN_HEIGHT+0, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+1, x
	_bgt @no_match
	cmp _COLUMN_HEIGHT+2, x
	_bgt @no_match
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED_AND_OPEN
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED_AND_OPEN
	sta _GRID+0+0, y
	sta _GRID+0+1, y
	sta _GRID+0+2, y
	sta _GRID-8+1, y
	
	@match:
	lda #1
	ldx #0
	rts
	
	@no_match:
	lda #0
	tax
	rts
.endproc

