#ifndef PGM_H
#define PGM_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
	uint8_t r, g, b;
} color_t;

color_t *color_set(color_t *c, uint8_t r, uint8_t g, uint8_t b);

int pgm_write_header(FILE *fp, unsigned int w, unsigned int h);
int pgm_write_pixel(FILE *fp, color_t *c);

#endif
