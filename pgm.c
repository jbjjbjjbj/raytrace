#include "pgm.h"

#include <stdio.h>
#include <stdlib.h>

#define COLOR_MAX 255

int pgm_write_header(FILE *fp, unsigned int w, unsigned int h)
{
	return fprintf(fp, "P3\n%d %d\n%d\n", w, h, COLOR_MAX);
}

int pgm_write_pixel(FILE *fp, color_t *c)
{
	return fprintf(fp, "%.0lf %.0lf %.0lf\n", c->r * COLOR_MAX, c->g * COLOR_MAX, c->b * COLOR_MAX);
}

color_t *color_set(color_t *c, uint8_t r, uint8_t g, uint8_t b)
{
	if (!c) {
		c = (color_t *) malloc(sizeof(color_t));
	}

	c->r = r;
	c->g = g;
	c->b = b;

	return c;
}

color_t *color_add(color_t *dest, color_t *a, color_t *b)
{
	if (!dest) {
		dest = (color_t *) malloc(sizeof(dest));
	}

	COORD_T tmp = a->r + b->r;
	dest->r = tmp > 1 ? 1 : tmp;

	tmp = a->g + b->g;
	dest->g = tmp > 1 ? 1 : tmp;

	tmp = a->b + b->b;
	dest->b = tmp > 1 ? 1 : tmp;

	return dest;
}

color_t *color_scale(color_t *dest, color_t *a, COORD_T b)
{
	if (!dest) {
		dest = (color_t *) malloc(sizeof(dest));
	}

	COORD_T tmp = a->r * b;
	dest->r = tmp > 1 ? 1 : tmp;

	tmp = a->g * b;
	dest->g = tmp > 1 ? 1 : tmp;

	tmp = a->b * b;
	dest->b = tmp > 1 ? 1 : tmp;

	return dest;
}

color_t *color_scale_vector(color_t *dest, color_t *a, vector_t *v)
{
	if (!dest) {
		dest = (color_t *) malloc(sizeof(dest));
	}

	dest->r = a->r * v->x;
	dest->g = a->g * v->y;
	dest->b = a->b * v->z;

	return dest;
}
