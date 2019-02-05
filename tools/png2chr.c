#include <string.h>
#include <alloca.h>

#include <png.h>

#include "slib.h"

typedef struct {
	uint w, h;
	u8 *pixels;
} Image;

typedef struct {
	u8 bytes[16];
} Tile;

static inline void pixel_to_chr(u8 pixel, u8 *output, u8 bit){
	if(pixel & 1) output[0] |= bit;
	if(pixel & 2) output[8] |= bit;
}

static Tile tile_to_chr(const u8 *pixels, uint stride){
	Tile tile = {};
	
	for(uint y = 0; y < 8; y++) {
		const u8 *row = pixels + y*stride;
		pixel_to_chr(row[0], tile.bytes + y, 0x80);
		pixel_to_chr(row[1], tile.bytes + y, 0x40);
		pixel_to_chr(row[2], tile.bytes + y, 0x20);
		pixel_to_chr(row[3], tile.bytes + y, 0x10);
		pixel_to_chr(row[4], tile.bytes + y, 0x08);
		pixel_to_chr(row[5], tile.bytes + y, 0x04);
		pixel_to_chr(row[6], tile.bytes + y, 0x02);
		pixel_to_chr(row[7], tile.bytes + y, 0x01);
	}
	
	return tile;
}

int main(int argc, char **argv){
	SLIB_ASSERT_HARD(argc == 3, "Usage: %s infile outfile", argv[0]);
	
	FILE *infile = fopen(argv[1], "r");
	SLIB_ASSERT_HARD(infile, "Can't open input file '%s'", argv[1]);
	
	FILE *outfile = fopen(argv[2], "w");
	SLIB_ASSERT_HARD(outfile, "Can't open output file '%s'", argv[2]);
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	SLIB_ASSERT_HARD(png_ptr, "PNG error");
	
	png_infop info = png_create_info_struct(png_ptr);
	SLIB_ASSERT_HARD(info, "PNG error");
	
	if(setjmp(png_jmpbuf(png_ptr))) SLIB_ABORT("PNG error");
	
	png_init_io(png_ptr, infile);
	png_read_png(png_ptr, info, PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_STRIP_ALPHA, NULL);
	
	Image image = {};
	
	image.w = png_get_image_width(png_ptr, info);
	image.h = png_get_image_height(png_ptr, info);
	image.pixels = alloca(image.w * image.h);
	
	SLIB_ASSERT_HARD(image.w%8 == 0 && image.h%8 == 0, "Image dimensions must be divisible by 8.");
	SLIB_ASSERT_HARD(png_get_color_type(png_ptr, info) == PNG_COLOR_TYPE_PALETTE, "Image must use a palette.");
	SLIB_ASSERT_HARD(png_get_bit_depth(png_ptr, info) == 8, "Image is not 8bpp.");
	
	const u32 rowbytes = png_get_rowbytes(png_ptr, info);
	SLIB_ASSERT_HARD(rowbytes == image.w, "Packing failed, row was %u instead of %u", rowbytes, image.w);
	
	u8 **png_rows = png_get_rows(png_ptr, info);
	for(uint i = 0; i < image.h; i++) memcpy(image.pixels + image.w*i, png_rows[i], image.w);
	
	png_destroy_info_struct(png_ptr, &info);
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	
	uint rows = image.h/8;
	uint cols = image.w/8;
	
	SLIB_LOG("%ux%u png to %u tiles.", image.w, image.h, rows * cols);
	
	for(u32 r = 0; r < image.h/8; r++) {
		for(u32 c = 0; c < image.w/8; c++) {
			const u8 *pixels = image.pixels + 8*(c + r*image.w);
			Tile tile = tile_to_chr(pixels, image.w);
			fwrite(&tile, sizeof(tile), 1, outfile);
		}
	}
	
	return EXIT_SUCCESS;
}
