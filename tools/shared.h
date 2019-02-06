#include <stdio.h>

#include "slib.h"

typedef struct {
	uint w, h;
	u8 *pixels;
} Image;

Image read_png(FILE *infile);
