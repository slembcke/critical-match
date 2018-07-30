#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef uint8_t u8;

typedef struct {
	u8 r, g, b, a;
} Pixel;

#define TILE_SIZE 8

typedef struct{
	Pixel pixels[TILE_SIZE*TILE_SIZE];
} Tile;

typedef struct {
	int width, height, stride;
	Pixel *pixels;
} Image;

static void blit_tile(Tile *tile, Image *image, unsigned x, unsigned y){
	Pixel *origin = image->pixels + x + y*image->stride;
	
	for(unsigned row = 0; row < TILE_SIZE; row++){
		memcpy(tile->pixels + row*TILE_SIZE, origin + row*image->stride, sizeof(Pixel)*TILE_SIZE);
	}
}

int main(void){
	Image image;
	
	int channels;
	image.pixels = (Pixel *)stbi_load("gfx/bg.png", &image.width, &image.height, &channels, 4);
	printf("%d x %d @ %d channels\n", image.width, image.height, channels);
	
	Tile a, b;
	blit_tile(&a, &image, 0, 0);
	blit_tile(&a, &image, 8, 0);
	
	printf("%d\n", memcmp(a.pixels, b.pixels, sizeof(Tile)));
	
	printf("Tile count: %d", 12);
	return EXIT_SUCCESS;
}
