#ifndef PGM_H
#define PGM_H

#include <stdio.h>
#include <stdint.h>
#include "vector.h"

typedef struct {
	COORD_T r, g, b;
} color_t;

void color_clamp(color_t *c);

void color_gamma_correct(color_t *c);
color_t *color_set(color_t *c, COORD_T r, COORD_T g, COORD_T b);
color_t *color_scale(color_t *dest, color_t *a, COORD_T b);
color_t *color_add(color_t *dest, color_t *a, color_t *b);
color_t *color_scale_vector(color_t *dest, color_t *a, vector_t *v);

int pgm_write_header(FILE *fp, unsigned int w, unsigned int h);
int pgm_write_pixel(FILE *fp, color_t *c);

#endif
