.macpack generic

.importzp _ix, _iy, _idx
.import _GRID, _COLUMN_HEIGHT

BLOCK_STATUS_UNLOCKED = $20

; x: _ix, y: _iy, block index: _idx
.export _grid_match_tetromino
.proc _grid_match_tetromino
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
	
	; Unlock matched blocks. |= BLOCK_STATUS_UNLOCKED
	lda _GRID, y
	ora #BLOCK_STATUS_UNLOCKED
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
