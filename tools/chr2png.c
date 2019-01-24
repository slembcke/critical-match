#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <png.h>

#include "common.h"

// http://wiki.nesdev.com/w/index.php/PPU_palettes
static const png_color NES_PALETTE[] = {
	{ 84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, { 48,   0, 136}, { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, { 60,  24,   0}, { 32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, {  0,  60,   0}, {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, { 92,  30, 228}, {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, {120,  60,   0}, { 84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40}, {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236}, {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, {212, 136,  32}, {160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108}, { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {  0,   0,   0},
};

static void savepng(FILE *f, const u8 data[], const u32 tile_count, uint8_t palette[]){
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) die("PNG error\n");
	
	png_infop info = png_create_info_struct(png_ptr);
	if(!info) die("PNG error\n");
	if(setjmp(png_jmpbuf(png_ptr))) die("PNG error\n");
	
	// Calculate a suitable resolution.
	u32 w, h;
	if(tile_count < 16){
		h = 8;
		w = 8*tile_count;
	} else if(tile_count % 16){
		h = 8*tile_count/16 + 1;
		w = 8*16;
	} else {
		h = 8*tile_count/16;
		w = 8*16;
	}
	
	printf("Saving to a %ux%u PNG.\n", w, h);
	
	png_init_io(png_ptr, f);
	png_set_IHDR(png_ptr, info, w, h, 2, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	const png_color colors[] = {NES_PALETTE[palette[0]], NES_PALETTE[palette[1]], NES_PALETTE[palette[2]], NES_PALETTE[palette[3]]};
	png_set_PLTE(png_ptr, info, colors, 4);
	
	png_write_info(png_ptr, info);
	png_set_packing(png_ptr);

	// Convert to a single buffer
	u8 *pdata = calloc(w * h, 1);
	const u8 *ptr = data;
	for(u32 i = 0; i < tile_count; i++){
		const u32 row = (i / 16) * 8;
		const u32 col = (i % 16) * 8;
		u8 rows[8][8] = {};
			
		for(u32 j = 0; j < 8; j++){
			const u8 pix = *ptr;
			ptr++;
			
			if(pix & 0x80) rows[j][0] |= 1;
			if(pix & 0x40) rows[j][1] |= 1;
			if(pix & 0x20) rows[j][2] |= 1;
			if(pix & 0x10) rows[j][3] |= 1;
			if(pix & 0x08) rows[j][4] |= 1;
			if(pix & 0x04) rows[j][5] |= 1;
			if(pix & 0x02) rows[j][6] |= 1;
			if(pix & 0x01) rows[j][7] |= 1;
		}
		
		for(u32 j = 0; j < 8; j++){
			const u8 pix = *ptr;
			ptr++;
			
			if(pix & 0x80) rows[j][0] |= 2;
			if(pix & 0x40) rows[j][1] |= 2;
			if(pix & 0x20) rows[j][2] |= 2;
			if(pix & 0x10) rows[j][3] |= 2;
			if(pix & 0x08) rows[j][4] |= 2;
			if(pix & 0x04) rows[j][5] |= 2;
			if(pix & 0x02) rows[j][6] |= 2;
			if(pix & 0x01) rows[j][7] |= 2;
		}
		
		// Copy over
		for (u32 j = 0; j < 8; j++){
			memcpy(pdata + (row + j) * w + col, &rows[j][0], 8);
		}
	}

	// Write
	for(u32 i = 0; i < h; i++) png_write_row(png_ptr, pdata + i * w);
	
	png_write_end(png_ptr, NULL);
	free(pdata);

	png_destroy_info_struct(png_ptr, &info);
	png_destroy_write_struct(&png_ptr, NULL);
}

int main(int argc, char *argv[]){
	if(argc != 4) die("Usage: %s palette infile.chr outfile.png\n", argv[0]);
	
	FILE *infile = fopen(argv[2], "r");
	if(!infile) die("Can't open file\n");
	
	FILE *outfile = fopen(argv[3], "w");
	if(!outfile) die("Can't open output file '%s'\n", argv[3]);
	
	struct stat st;
	fstat(fileno(infile), &st);
	if (st.st_size % 16 || !st.st_size) die("Unaligned CHR file?\n");
	
	u8 *data = calloc(st.st_size, 1);
	fread(data, st.st_size, 1, infile);
	
	u8 pal[4];
	sscanf(argv[1], "%02hhx %02hhx %02hhx %02hhx", pal + 0, pal + 1, pal + 2, pal + 3);
	
	savepng(outfile, data, st.st_size / 16, pal);
	
	return EXIT_SUCCESS;
}
