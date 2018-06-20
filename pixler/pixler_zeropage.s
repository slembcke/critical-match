.exportzp px_ctrl
.exportzp px_ticks, _px_ticks = px_ticks
.exportzp _PX = PX
.exportzp PX_scroll_x
.exportzp PX_scroll_y
.exportzp PX_buffer

.zeropage

px_ctrl: .byte 0

px_ticks: .byte 0

PX:
PX_scroll_x: .word 0
PX_scroll_y: .word 0
PX_buffer: .addr 0
