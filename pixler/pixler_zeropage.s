.exportzp px_mask, _px_mask = px_mask
.exportzp px_ctrl, _px_ctrl = px_ctrl
.exportzp px_ticks, _px_ticks = px_ticks
.exportzp px_sprite_cursor
.exportzp _PX = PX
.exportzp PX_scroll_x
.exportzp PX_scroll_y
.exportzp PX_buffer

.zeropage

px_mask: .byte 0
px_ctrl: .byte 0

px_ticks: .byte 0

px_sprite_cursor: .byte 0

PX:
PX_scroll_x: .word 0
PX_scroll_y: .word 0
PX_buffer: .addr 0
