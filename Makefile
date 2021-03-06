PROJECT_NAME = critical-match
ROM = $(PROJECT_NAME).nes

CC65_ROOT = ../cc65
CC = $(CC65_ROOT)/bin/cc65
AS = $(CC65_ROOT)/bin/ca65
LD = $(CC65_ROOT)/bin/ld65
AR = $(CC65_ROOT)/bin/ar65

CFLAGS = -t nes -Oirs --register-space 16
# CFLAGS += -DDEBUG

INCLUDE = -I . -I $(CC65_ROOT)/include -I lib
ASMINC = -I lib/ -I $(CC65_ROOT)/libsrc/nes

SRC = \
	src/main.c \
	src/grid.c \
	src/player.c \
	src/debug.c \

ASMSRC = \
	src/zeropage.s \
	src/match.s \
	src/sprites.s \
	src/metatiles.s \
	gfx/gfx.s \
	audio/audio.s \
	lib/famitone2/famitone2.s \

OBJS = $(ASMSRC:.s=.o) $(SRC:.c=.o)

PX_SRC = \
	lib/pixler/coroutine.s \
	lib/pixler/rand8.s \
	lib/pixler/boot.s \
	lib/pixler/zeropage.s \
	lib/pixler/nmi.s \
	lib/pixler/wait_frames.s \
	lib/pixler/uxrom.s \
	lib/pixler/buffer.s \
	lib/pixler/buffer_inc.s \
	lib/pixler/buffer_color.s \
	lib/pixler/buffer_data.s \
	lib/pixler/inc.s \
	lib/pixler/fill.s \
	lib/pixler/blit.s \
	lib/pixler/sprite.s \
	lib/pixler/lz4.s \
	lib/pixler/lz4_to_vram.s \
	lib/pixler/lz4_to_ram.s \

PX_OBJS = $(PX_SRC:.s=.o)
PX_LIB = lib/pixler/px.lib

GFX = \
	gfx/CHR0.png \
	gfx/explosion.png \
	gfx/character.png \

PAL0 = "02 07 1A 14"
PAL1 = "02 17 28 20"
PAL2 = "02 11 22 32"
PAL3 = "02 28 38 20"

MAPS = \
	gfx/menu.tmx \
	gfx/credits.tmx \
	gfx/board.tmx \
	gfx/game_over.tmx

SONGS = \
	audio/gameplay2.txt

.PHONY: default clean rom run-mac run-linux itch

default: rom

clean:
	rm -rf $(ROM)
	rm -rf $(SRC:.c=.s)
	rm -rf $(OBJS)
	rm -rf $(PX_OBJS)
	rm -rf gfx/*.chr
	rm -rf gfx/*.lz4chr
	rm -rf gfx/shapes.bin
	rm -rf gfx/*.lz4
	rm -rf gfx/*-pal?.png
	rm -rf $(SONGS:.txt=.s)
	rm -rf link.log
	make -C tools clean

rom: $(ROM)

run-mac: $(ROM)
	open -a Nestopia $(ROM)

run-linux: $(ROM)
	nestopia -w -l 1 -n -s 2 -t $(ROM)

run-win: $(ROM)
	../Mesen/Mesen.exe $(ROM)

tools/png2chr:
	make -C tools png2chr

tools/chr2png:
	make -C tools chr2png

tools/lz4x:
	make -C tools lz4x

tools/text2data:
	make -C tools text2data

$(PX_LIB): $(PX_OBJS)
	$(AR) r $@ $^

$(ROM): ld65.cfg $(OBJS) $(PX_LIB)
	$(LD) -C ld65.cfg $(OBJS) $(PX_LIB) nes.lib -m link.log -o $@

%.s: %.c src/shared.h
	$(CC) $(CFLAGS) $< --add-source $(INCLUDE) -o $@

%.o: %.s
	$(AS) $< $(ASMINC) -o $@

%.chr: %.png tools/png2chr
	tools/png2chr $< $@

%.lz4chr: %.chr tools/lz4x
	tools/lz4x -f9 $< $@
	touch $@

%.bin: %.hex
	xxd -r $< > $@

%.bin: %.tmx
	python tools/tmx2bin.py $< > $@

%.lz4: %.bin tools/lz4x
	tools/lz4x -f9 $< $@
	touch $@

gfx/gfx.o: $(GFX:.png=.lz4chr) $(MAPS:.tmx=.lz4) gfx/shapes.bin

audio/sounds.s: audio/sounds.nsf tools/nsf2data
	tools/nsf2data $< -ca65 -ntsc

audio/%.s: audio/%.txt tools/text2data
	tools/text2data -ca65 $<

audio/audio.o: $(SONGS:.txt=.s) audio/sounds.s

dat/data.o:

ITCH_DIR = /tmp/$(PROJECT_NAME)
itch: rom
	rm -rf $(ITCH_DIR) $(ITCH_DIR).zip
	mkdir -p $(ITCH_DIR)
	cp $(ROM) $(ITCH_DIR)
	cp jsnes/itch.html $(ITCH_DIR)/index.html
	cp jsnes/nes-embed.js $(ITCH_DIR)
	zip -rj $(ITCH_DIR).zip $(ITCH_DIR)

tiles: gfx/CHR0.chr tools/chr2png
	tools/chr2png $(PAL0) $< $(<:.chr=-pal0.png)
	tools/chr2png $(PAL1) $< $(<:.chr=-pal1.png)
	tools/chr2png $(PAL2) $< $(<:.chr=-pal2.png)
	tools/chr2png $(PAL3) $< $(<:.chr=-pal3.png)

# Cancel built in rule for .c files.
%.o: %.c
