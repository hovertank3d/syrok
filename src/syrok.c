/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <cyb3r@nigge.rs> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Mykhailo Chernysh
 * ----------------------------------------------------------------------------
 */


#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "error.h"

#define STBI_NO_LINEAR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "syrok.h"

typedef struct pixel {
	uint8_t r, g, b, a;
} __attribute__((packed)) pixel;

typedef struct syrok_image {
	pixel  *data;
	int 	w, h;
	int 	comp;
} syrok_image;

typedef pixel (*syrok_hadler)(pixel, int, int);

static pixel syrok_monochrome(pixel p, int x, int y);
static pixel syrok_colored(pixel p, int x, int y);
static pixel syrok_colored_xor(pixel p, int x, int y);
static pixel syrok_colored_and(pixel p, int x, int y);
static pixel syrok_colored_or(pixel p, int x, int y);

syrok_hadler syrok_handlers[] = {
	syrok_monochrome,
	syrok_colored,
	syrok_colored_xor,
	syrok_colored_and,
	syrok_colored_or,
};

static pixel syrok_pixel(pixel p, int x, int y)
{
	pixel 		result;
	uint32_t 	level;
	uint32_t 	pattern;
	uint32_t 	sum;
	int   	    and;

	sum = p.r + p.g + p.b;
	
	level = (sum/3) >> 4;
	and = level < 8;
	pattern = and ? x&y : x^y;

	pattern <<= 8 - (level + 1) / 2;

	result.r = pattern;
	result.g = pattern;
	result.b = pattern;

	result.a = p.a;
	return result;
}

static pixel syrok_monochrome(pixel p, int x, int y)
{
	return syrok_pixel(p, x, y);
}

static pixel syrok_colored(pixel p, int x, int y)
{
	pixel result;
	pixel mono;
	double r1, g1, b1;
	double r2, g2, b2;

	mono = syrok_pixel(p, x, y);

	r1 = p.r / 255.0;
	g1 = p.g / 255.0;
	b1 = p.b / 255.0;
	r2 = (uint8_t)(mono.r) / 255.0;
	g2 = (uint8_t)(mono.g) / 255.0;
	b2 = (uint8_t)(mono.b) / 255.0;

	result.r = (uint8_t)(r1 * r2 * 255.0);
	result.g = (uint8_t)(g1 * g2 * 255.0);
	result.b = (uint8_t)(b1 * b2 * 255.0);
	result.a = p.a;

	return result;
}

static pixel syrok_colored_xor(pixel p, int x, int y)
{
	pixel mono;
	pixel result;

	mono = syrok_pixel(p, x, y);
	result.r = mono.r ^ p.r;
	result.g = mono.g ^ p.g;
	result.b = mono.b ^ p.b;
	result.a = p.a;
	return result;
}

static pixel syrok_colored_and(pixel p, int x, int y)
{
	pixel mono;
	pixel result;

	mono = syrok_pixel(p, x, y);
	result.r = mono.r & p.r;
	result.g = mono.g & p.g;
	result.b = mono.b & p.b;
	result.a = p.a;
	return result;
}

static pixel syrok_colored_or(pixel p, int x, int y)
{
	pixel mono;
	pixel result;

	mono = syrok_pixel(p, x, y);
	result.r = mono.r | p.r;
	result.g = mono.g | p.g;
	result.b = mono.b | p.b;
	result.a = p.a;
	return result;
}

static uint8_t *read_file(const char *filename, size_t *size)
{
	FILE *f;
	uint8_t *data;
	size_t read_bytes;

	f = fopen(filename, "rb");
	if (f == NULL) {
		syrok_set_error("Can't read image: %s", strerror(errno));
		return NULL;
	}

	fseek(f, 0L, SEEK_END);
	*size = ftell(f);
	rewind(f);

	data = malloc(*size);
	if (data == NULL) {
		syrok_set_error("Out of memory");
		return NULL;
	}

	read_bytes = fread(data, 1, *size, f);

	if (read_bytes != *size) {
		syrok_set_error("Reading error");
		return NULL;
	}

	return data;
}

static int read_image(const uint8_t *data, size_t data_size, syrok_image *img)
{
	img->data = (pixel *)stbi_load_from_memory(data, data_size, &img->w, &img->h, &img->comp, 4);
	if (img->data == NULL) {
		syrok_set_error("Can't parse image: %s", stbi_failure_reason());
		return 0;
	}
	return 1;
}

uint8_t *syrok_read_file(const char *filename, int *retsize, int mode)
{
	uint8_t *result_buffer;
	uint8_t *img_file_data;
	size_t img_file_size;

	img_file_data = read_file(filename, &img_file_size);
	if (img_file_data == NULL) {
		return NULL;
	}

	result_buffer = syrok(img_file_data, img_file_size, retsize, mode);
	free(img_file_data);

	return result_buffer;
}

uint8_t *syrok(const uint8_t *data, size_t filesize, int *retsize, int mode)
{
	syrok_hadler func;
	syrok_image result_image;
	syrok_image img;
	uint8_t *compressed_image;
	int x, y;

	if (mode < 0 || mode >= SYROK_MODES_COUNT) {
		syrok_set_error("Invalid mode: %02x", mode);
		return NULL;
	}

	if (!read_image(data, filesize, &img)) {
		return NULL;
	}

	result_image.w = img.w;
	result_image.h = img.h;
	result_image.comp = 4;
	result_image.data = malloc(sizeof(pixel) * img.w * img.h);

	if (result_image.data == NULL) {
		syrok_set_error("Out of memory");
		free(img.data);
		return NULL;
	}

	func = syrok_handlers[mode];

	for (x = 0; x < img.w; x++) {
		for (y = 0; y < img.h; y++) {
			result_image.data[x + y * img.w] = func(img.data[x + y * img.w], x, y);
		}
	}
	free(img.data);

	compressed_image = stbi_write_png_to_mem((uint8_t *)result_image.data, img.w * sizeof(pixel),
											 result_image.w, result_image.h,
											 result_image.comp, retsize);

	if (compressed_image == NULL) {
		syrok_set_error("Can't encode image: %s", stbi_failure_reason());
		return NULL;
	}

	free(result_image.data);

	return compressed_image;
}

