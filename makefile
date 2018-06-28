PROJECT_NAME = TreasureStack
ROM = $(PROJECT_NAME).nes

CC65_ROOT = ../cc65
CC = $(CC65_ROOT)/bin/cc65
AS = $(CC65_ROOT)/bin/ca65
LD = $(CC65_ROOT)/bin/ld65

CFLAGS = -t nes -Oirs --register-space 16

INCLUDE = -I $(CC65_ROOT)/include -I pixler
ASMINC = -I . -I $(CC65_ROOT)/libsrc/nes

SRC = \
	src/main.c \
	src/grid.c \
	src/player.c \
	src/debug.c \
	pixler/pixler.c

ASMSRC = \
	src/zeropage.s \
	src/sprites.s \
	src/audio.s \
	src/metatiles.s \
	data/data.s \
	pixler/pixler_boot.s \
	pixler/pixler_zeropage.s \
	pixler/pixler_nmi.s \
	pixler/pixler_banks.s \
	pixler/pixler_buffer.s \
	pixler/pixler_blit.s \
	pixler/pixler_sprite.s \
	famitone2/famitone2.s

GFX = \
	gfx/sheet1.chr \
	gfx/squidman.chr

OBJS = $(ASMSRC:.s=.o) $(SRC:.c=.o)

rom: $(ROM)

$(ROM): ld65.cfg $(OBJS)
	$(LD) -C ld65.cfg $(OBJS) nes.lib -m link.log -o $@

clean:
	rm -rf $(ROM)
	rm -rf $(SRC:.c=.s)
	rm -rf $(OBJS)
	rm -rf gfx/*.chr
	rm -rf link.log

run-mac: $(ROM)
	open -a Nestopia $(ROM)

run-linux: $(ROM)
	nestopia -d -w -l 0 -n -s 1 -t $(ROM)

%.s: %.c
	$(CC) $(CFLAGS) $< --add-source $(INCLUDE) -o $@

%.o: %.s
	$(AS) $< $(ASMINC) -o $@

%.chr: %.png
	tools/png2chr $< $@

pixler/pixler_banks.o: $(GFX)

data/grid.bin: data/grid.hex
	xxd -r -c 8 $< $@

# Cancel built in rule for .c files.
%.o: %.c
