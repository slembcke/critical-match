#include <string.h>

#include <png.h>

#include "slib.h"

typedef struct {
	uint w, h;
	u8 *pixels;
} Image;

int main(int argc, char **argv){
	SLIB_ASSERT_HARD(argc == 3, "Usage: %s infile outfile\n", argv[0]);
	
	FILE *infile = fopen(argv[1], "r");
	SLIB_ASSERT_HARD(infile, "Can't open input file '%s'\n", argv[1]);
	
	FILE *outfile = fopen(argv[2], "w");
	SLIB_ASSERT_HARD(outfile, "Can't open output file '%s'\n", argv[2]);
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	SLIB_ASSERT_HARD(png_ptr, "PNG error\n");
	
	png_infop info = png_create_info_struct(png_ptr);
	SLIB_ASSERT_HARD(info, "PNG error\n");
	
	if(setjmp(png_jmpbuf(png_ptr))) SLIB_ABORT("PNG error\n");
	
	png_init_io(png_ptr, infile);
	png_read_png(png_ptr, info, PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_STRIP_ALPHA, NULL);
	
	Image image = {};
	image.w = png_get_image_width(png_ptr, info);
	image.h = png_get_image_height(png_ptr, info);
	image.pixels = alloca(image.w * image.h);
	
	SLIB_ASSERT_HARD(image.w%8 == 0 && image.h%8 == 0, "Image dimensions must be divisible by 8.\n");
	SLIB_ASSERT_HARD(png_get_color_type(png_ptr, info) == PNG_COLOR_TYPE_PALETTE, "Image must use a palette.\n");
	SLIB_ASSERT_HARD(png_get_bit_depth(png_ptr, info) == 8, "Image is not 8bpp.\n");
	
	const u32 rowbytes = png_get_rowbytes(png_ptr, info);
	SLIB_ASSERT_HARD(rowbytes == image.w, "Packing failed, row was %u instead of %u\n", rowbytes, image.w);
	
	u8 **png_rows = png_get_rows(png_ptr, info);
	for(uint i = 0; i < image.h; i++) memcpy(image.pixels + image.w*i, png_rows[i], image.w);
	
	png_destroy_info_struct(png_ptr, &info);
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	
	uint rows = image.h/8;
	uint cols = image.w/8;
	
	SLIB_LOG("Converting %ux%u PNG to %u CHR tiles.\n", image.w, image.h, rows * cols);
	
	for(u32 r = 0; r < rows; r++) {
		for(u32 c = 0; c < cols; c++) {
			u8 buf0[8] = {}, buf1[8] = {};
			for(u32 y = r * 8; y < r * 8 + 8; y++) {
				for(u32 x = c * 8; x < c * 8 + 8; x++) {
					const u8 pix = image.pixels[x + y*image.w];
					if(pix & 1) buf0[y % 8] |= 1 << (7 - x%8);
					if(pix & 2) buf1[y % 8] |= 1 << (7 - x%8);
				}
			}
			
			fwrite(buf0, 8, 1, outfile);
			fwrite(buf1, 8, 1, outfile);
		}
	}
	
	return EXIT_SUCCESS;
}
