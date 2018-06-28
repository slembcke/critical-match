.include "pixler/pixler.inc"

.export METATILE0, METATILE1, METATILE2, METATILE3, METATILE4

.rodata ; TODO move this somewhere else.

; Empty, Chest, Open, Key
METATILE0: .byte  $00,  $00,  $00,  $00,  $80,  $82,  $84,  $86,  $8A,  $88,  $88,  $88,  $8C,  $8C,  $8C,  $8C
METATILE1: .byte  $00,  $00,  $00,  $00,  $81,  $83,  $85,  $87,  $89,  $8B,  $89,  $89,  $8D,  $8D,  $8D,  $8D
METATILE2: .byte  $00,  $00,  $00,  $00,  $90,  $92,  $94,  $96,  $98,  $98,  $9A,  $98,  $92,  $92,  $92,  $92
METATILE3: .byte  $00,  $00,  $00,  $00,  $91,  $93,  $91,  $97,  $99,  $99,  $99,  $9B,  $93,  $93,  $93,  $93
METATILE4: .byte PAL0, PAL0, PAL0, PAL0, PAL0, PAL1, PAL2, PAL3, PAL0, PAL1, PAL2, PAL3, PAL0, PAL1, PAL2, PAL3
