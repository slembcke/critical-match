#include <string.h>
#include <alloca.h>

#include "shared.h"

// Convert a row of 8 pixels to a CHR byte.
static inline u8 row_to_chr(const void *pixels, uint shift){
	// Load 64 bits / 8 bytes for the whole row of the tile.
	// Shift so the bit we want is in the top bit of each byte and mask.
	// Mod 511 (2**9 - 1) selects each bit per 9 bit sequence to collect them into a byte.
	// In combinatino with the earlier shift, this reverses the byte -> bit mapping.
	return ((*(u64 *)pixels << shift) & 0x8080808080808080) % 511;
}

static void tile_to_chr(const u8 *pixels, uint stride, u8 tile[]){
	for(uint i = 0; i < 8; i++) tile[i + 0] = row_to_chr(pixels + i*stride, 7);
	for(uint i = 0; i < 8; i++) tile[i + 8] = row_to_chr(pixels + i*stride, 6);
}

int main(int argc, char **argv){
	SLIB_ASSERT_HARD(argc == 3, "Usage: %s infile outfile", argv[0]);
	
	FILE *infile = fopen(argv[1], "r");
	SLIB_ASSERT_HARD(infile, "Can't open input file '%s'", argv[1]);
	
	FILE *outfile = fopen(argv[2], "w");
	SLIB_ASSERT_HARD(outfile, "Can't open output file '%s'", argv[2]);
	
	Image image = read_png(infile);	
	for(uint r = 0; r < image.h/8; r++) {
		for(uint c = 0; c < image.w/8; c++) {
			const u8 *pixels = image.pixels + 8*(c + r*image.w);
			u8 tile[16] = {};
			tile_to_chr(pixels, image.w, tile);
			fwrite(&tile, sizeof(tile), 1, outfile);
		}
	}
	
	return EXIT_SUCCESS;
}
