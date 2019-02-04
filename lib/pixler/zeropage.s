.zeropage

.exportzp px_mask, _px_mask = px_mask
px_mask: .res 1

.exportzp px_ctrl, _px_ctrl = px_ctrl
px_ctrl: .res 1

.exportzp px_nmi_ready
px_nmi_ready: .res 1

.exportzp px_ticks, _px_ticks = px_ticks
px_ticks: .res 1

.exportzp px_sprite_cursor
px_sprite_cursor: .res 1

.exportzp px_buffer_cursor
px_buffer_cursor: .res 1

.exportzp _PX
_PX:
.exportzp PX_scroll_x
PX_scroll_x: .res 2
.exportzp PX_scroll_y
PX_scroll_y: .res 2
.exportzp PX_buffer
PX_buffer: .res 2
