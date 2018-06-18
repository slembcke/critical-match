.exportzp px_ctrl
.exportzp _PX = PX
.exportzp PX_scroll_x
.exportzp PX_scroll_y
.exportzp PX_buffer

.zeropage

px_ctrl: .res 1

PX:
PX_scroll_x: .res 2
PX_scroll_y: .res 2
PX_buffer: .res 2
