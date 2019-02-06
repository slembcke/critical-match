CFLAGS += -std=c99 -Wall -Wextra -I /usr/local/include

.phony: all clean

all: png2chr chr2png text2data nsf2data lz4x png2tiles

clean:
	rm -rf *.o *.dSYM png2chr chr2png text2data nsf2data lz4x

slib.o: slib.h

png2chr: png2chr.o slib.o shared.o
	$(CC) -o $@ $^ $(CFLAGS) -lpng

chr2png: chr2png.o slib.o
	$(CC) -o $@ $^ $(CFLAGS) -lpng

text2data: text2data.cpp
	$(CC) -o $@ $<

nsf2data: nsf2data.cpp
	$(CC) -o $@ $<

lz4x: lz4x.cpp
	$(CC) -o $@ $<

png2tiles: png2tiles.c
	$(CC) -o $@ $<
