PROJECT_NAME = TreasureStack
ROM = $(PROJECT_NAME).nes

CC65_ROOT = ../cc65
CC = $(CC65_ROOT)/bin/cc65
AS = $(CC65_ROOT)/bin/ca65
LD = $(CC65_ROOT)/bin/ld65

CFLAGS = -t nes -Oirs

INCLUDE = -I $(CC65_ROOT)/include -I pixler
ASMINC = $(CC65_ROOT)/libsrc/nes

SRC = \
	src/main.c \
	src/grid.c \
	src/debug.c \
	pixler/pixler.c

ASMSRC = \
	src/zeropage.s \
	pixler/pixler_boot.s \
	pixler/pixler_zeropage.s \
	pixler/pixler_nmi.s \
	pixler/pixler_banks.s \
	pixler/pixler_buffer.s \
	pixler/pixler_blit.s

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
	nestopia -d -w -l 1 -n -s 4 -t $(ROM)

%.s: %.c
	$(CC) $(CFLAGS) $< --add-source $(INCLUDE) -o $@

%.o: %.s
	$(AS) $< -I $(ASMINC) -o $@

%.chr: %.png
	tools/png2chr $< $@

pixler/pixler_banks.o: $(GFX)

# Cancel built in rule for .c files.
%.o: %.c
