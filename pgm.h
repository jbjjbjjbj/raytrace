#ifndef PGM_H
#define PGM_H

#include <stdio.h>
#include <stdint.h>
#include "vector.h"

typedef struct {
	uint8_t r, g, b;
} color_t;

color_t *color_set(color_t *c, uint8_t r, uint8_t g, uint8_t b);
color_t *color_scale(color_t *dest, color_t *a, float b);
color_t *color_add(color_t *dest, color_t *a, color_t *b);
color_t *color_scale_vector(color_t *dest, color_t *a, vector_t *v);

int pgm_write_header(FILE *fp, unsigned int w, unsigned int h);
int pgm_write_pixel(FILE *fp, color_t *c);

#endif
