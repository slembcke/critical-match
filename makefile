PROJECT_NAME = TreasureStack
ROM = $(PROJECT_NAME).nes

CC65_ROOT = ../cc65
CC = $(CC65_ROOT)/bin/cc65
AS = $(CC65_ROOT)/bin/ca65
LD = $(CC65_ROOT)/bin/ld65

CFLAGS = -t nes -Oirs --register-space 16
# CFLAGS += -DDEBUG

INCLUDE = -I $(CC65_ROOT)/include -I lib
ASMINC = -I lib/ -I $(CC65_ROOT)/libsrc/nes

SRC = \
	src/main.c \
	src/grid.c \
	src/player.c \
	src/debug.c \
	lib/pixler/pixler.c

ASMSRC = \
	src/zeropage.s \
	src/sprites.s \
	src/audio.s \
	src/metatiles.s \
	gfx/gfx.s \
	dat/data.s \
	lib/coro/coro.s \
	lib/pixler/pixler_boot.s \
	lib/pixler/pixler_zeropage.s \
	lib/pixler/pixler_nmi.s \
	lib/pixler/pixler_banks.s \
	lib/pixler/pixler_buffer.s \
	lib/pixler/pixler_blit.s \
	lib/pixler/pixler_sprite.s \
	lib/pixler/lz4vram.s \
	lib/famitone2/famitone2.s

GFX = \
	gfx/neschar.png \
	gfx/sheet1.png \
	gfx/squidman.png

OBJS = $(ASMSRC:.s=.o) $(SRC:.c=.o)

.PHONY: clean rom run-mac run-linux

clean:
	rm -rf $(ROM)
	rm -rf $(SRC:.c=.s)
	rm -rf $(OBJS)
	rm -rf gfx/*.chr
	rm -rf gfx/*.lz4chr
	rm -rf dat/*.lz4
	rm -rf link.log
	make -C tools clean

rom: $(ROM)

run-mac: $(ROM)
	open -a Nestopia $(ROM)

run-linux: $(ROM)
	nestopia -d -w -l 0 -n -s 1 -t $(ROM)

tools/png2chr:
	make -C tools png2chr

tools/lz4x:
	make -C tools lz4x

$(ROM): ld65.cfg $(OBJS)
	$(LD) -C ld65.cfg $(OBJS) nes.lib -m link.log -o $@

%.s: %.c
	$(CC) $(CFLAGS) $< --add-source $(INCLUDE) -o $@

%.o: %.s
	$(AS) $< $(ASMINC) -o $@

%.chr: %.png tools/png2chr
	tools/png2chr $< $@

%.lz4chr: %.chr tools/lz4x
	tools/lz4x -9 $< $@

%.bin: %.hex
	xxd -r -c 8 $< > $@

%.lz4: %.bin tools/lz4x
	tools/lz4x -9 $< $@

gfx/gfx.s: $(GFX:.png=.lz4chr)

dat/data.s:

# Cancel built in rule for .c files.
%.o: %.c
