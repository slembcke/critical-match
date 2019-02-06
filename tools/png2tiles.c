#include <string.h>
#include <alloca.h>

#include "shared.h"

int main(int argc, char **argv){
	SLIB_ASSERT_HARD(argc == 3, "Usage: %s infile outfile", argv[0]);
	
	FILE *infile = fopen(argv[1], "r");
	SLIB_ASSERT_HARD(infile, "Can't open input file '%s'", argv[1]);
	
	FILE *outfile = fopen(argv[2], "w");
	SLIB_ASSERT_HARD(outfile, "Can't open output file '%s'", argv[2]);
	
	Image image = read_png(infile);
	uint rows = image.h/8;
	uint cols = image.w/8;
	uint tile_count = rows*cols;
	
	for(uint r = 0; r < image.h/8; r++) {
		for(uint c = 0; c < image.w/8; c++) {
			const u8 *pixels = image.pixels + 8*(c + r*image.w);
			// ... TODO
		}
	}
	
	return EXIT_SUCCESS;
}
