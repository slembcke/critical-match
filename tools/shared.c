#include <string.h>

#include <png.h>

#include "shared.h"

Image read_png(FILE *infile){
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
	image.pixels = malloc(image.w * image.h);
	
	SLIB_ASSERT_HARD(image.w%8 == 0 && image.h%8 == 0, "Image dimensions must be divisible by 8.");
	SLIB_ASSERT_HARD(png_get_color_type(png_ptr, info) == PNG_COLOR_TYPE_PALETTE, "Image must use a palette.");
	SLIB_ASSERT_HARD(png_get_bit_depth(png_ptr, info) == 8, "Image is not 8bpp.");
	
	const u32 rowbytes = png_get_rowbytes(png_ptr, info);
	SLIB_ASSERT_HARD(rowbytes == image.w, "Packing failed, row was %u instead of %u", rowbytes, image.w);
	
	u8 **png_rows = png_get_rows(png_ptr, info);
	for(uint i = 0; i < image.h; i++) memcpy(image.pixels + image.w*i, png_rows[i], image.w);
	
	png_destroy_info_struct(png_ptr, &info);
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	
	return image;
}
