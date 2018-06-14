PROJECT_NAME = TreasureStack
ROM = $(PROJECT_NAME).nes

CC65_ROOT = ../cc65
CC = $(CC65_ROOT)/bin/cc65
AS = $(CC65_ROOT)/bin/ca65
LD = $(CC65_ROOT)/bin/ld65

CFLAGS = -t nes -Oirs

INCLUDE = $(CC65_ROOT)/include
ASMINC = $(CC65_ROOT)/libsrc/nes

SRC = main.c pixler.c
ASMSRC = pixler_boot.s pixler_zeropage.s pixler_nmi.s pixler_banks.s pixler_buffer.s pixler_blit.s
OBJS = $(ASMSRC:.s=.o) $(SRC:.c=.o)

$(ROM): ld65.cfg $(OBJS)
	$(LD) -C ld65.cfg $(OBJS) nes.lib -m link.log -o $@

clean:
	git clean -fdX .

run: $(ROM)
	open -a Nestopia $(ROM)

%.s: %.c
	$(CC) $(CFLAGS) $< --add-source -I $(INCLUDE) -o $@

%.o: %.s
	$(AS) $< -I $(ASMINC) -o $@

# Cancel built in rule for .c files.
%.o: %.c
