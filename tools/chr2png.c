#include <alloca.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <png.h>

#include "slib.h"

// http://wiki.nesdev.com/w/index.php/PPU_palettes
static const png_color NES_PALETTE[] = {
	{ 84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, { 48,   0, 136}, { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, { 60,  24,   0}, { 32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, {  0,  60,   0}, {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, { 92,  30, 228}, {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, {120,  60,   0}, { 84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40}, {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236}, {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, {212, 136,  32}, {160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108}, { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {  0,   0,   0},
};

static void byte_to_row(u8 byte, u8 bit, u8 row[]){
	if(byte & 0x80) row[0] |= bit;
	if(byte & 0x40) row[1] |= bit;
	if(byte & 0x20) row[2] |= bit;
	if(byte & 0x10) row[3] |= bit;
	if(byte & 0x08) row[4] |= bit;
	if(byte & 0x04) row[5] |= bit;
	if(byte & 0x02) row[6] |= bit;
	if(byte & 0x01) row[7] |= bit;
}

int main(int argc, char *argv[]){
	SLIB_ASSERT_HARD(argc == 4, "Usage: %s palette infile.chr outfile.png", argv[0]);
	
	FILE *infile = fopen(argv[2], "r");
	SLIB_ASSERT_HARD(infile, "Can't open input file '%s'.", argv[2]);
	
	FILE *outfile = fopen(argv[3], "w");
	SLIB_ASSERT_HARD(outfile, "Can't open output file '%s'", argv[3]);
	
	struct stat st;
	fstat(fileno(infile), &st);
	SLIB_ASSERT_HARD(st.st_size > 0 && st.st_size%16 == 0, "Unaligned CHR file?");
	
	u8 *chr_data = alloca(st.st_size);
	fread(chr_data, st.st_size, 1, infile);
	
	u8 pal[4];
	sscanf(argv[1], "%02hhx %02hhx %02hhx %02hhx", pal + 0, pal + 1, pal + 2, pal + 3);
	
	// savepng(outfile, chr_data, st.st_size / 16, pal);
	uint tile_count = st.st_size/16;
	
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	SLIB_ASSERT_HARD(png_ptr, "PNG error");
	
	png_infop info = png_create_info_struct(png_ptr);
	SLIB_ASSERT_HARD(info, "PNG error");
	
	if(setjmp(png_jmpbuf(png_ptr))) SLIB_ABORT("PNG error");
	
	uint w = 128;
	uint h = 8*(tile_count + 15)/16;
	
	png_init_io(png_ptr, outfile);
	png_set_IHDR(png_ptr, info, w, h, 2, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_set_PLTE(png_ptr, info, (png_color[]){
		NES_PALETTE[pal[0]],
		NES_PALETTE[pal[1]],
		NES_PALETTE[pal[2]],
		NES_PALETTE[pal[3]]
	}, 4);
	
	png_write_info(png_ptr, info);
	png_set_packing(png_ptr);
	
	u8 *pixels = alloca(w*h);
	bzero(pixels, w*h);
	
	for(uint i = 0; i < tile_count; i++){
		const u8 *tile_bytes = chr_data + 16*i;
		
		uint row = 8*(i/16);
		uint col = 8*(i%16);
		u8 *tile_pixels = pixels + col + w*row;
		
		for(uint j = 0; j < 8; j++){
			byte_to_row(tile_bytes[j + 0], 1, tile_pixels + w*j);
			byte_to_row(tile_bytes[j + 8], 2, tile_pixels + w*j);
		}
	}

	for(uint i = 0; i < h; i++) png_write_row(png_ptr, pixels + i * w);
	
	png_write_end(png_ptr, NULL);

	png_destroy_info_struct(png_ptr, &info);
	png_destroy_write_struct(&png_ptr, NULL);
	
	return EXIT_SUCCESS;
}
