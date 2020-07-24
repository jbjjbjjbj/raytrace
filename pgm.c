#include "pgm.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define COLOR_MAX 255

int pgm_write_header(FILE *fp, unsigned int w, unsigned int h)
{
	return fprintf(fp, "P3\n%d %d\n%d\n", w, h, COLOR_MAX);
}

/*
 * Diden't really look that nice. 
void color_clamp(color_t *c)
{
	// Fint the max of the 3 values
	COORD_T max = c->r > c->g ? c->r : c->g;
	max = max > c->b ? max : c->b;

	if (max <= 1.0) {
		return;
	}

	// Scale everything by this
	color_scale(c, c, (COORD_T) 1 / max);
}
*/

#define MAXOUT(v) ( if (v > 1) { v = 1; } )

void color_clamp(color_t *c)
{
	// Just max them out
	if (c->r > 1) { c->r = 1; }
	if (c->g > 1) { c->g = 1; }
	if (c->b > 1) { c->b = 1; }
}

int pgm_write_pixel(FILE *fp, color_t *c)
{
    //color_gamma_correct(c);
	return fprintf(fp, "%.0lf %.0lf %.0lf\n", c->r * COLOR_MAX, c->g * COLOR_MAX, c->b * COLOR_MAX);
}

void color_gamma_correct(color_t *c) {
    c->r = sqrt(c->r);
    c->g = sqrt(c->g);
    c->b = sqrt(c->b);
}
color_t *color_set(color_t *c, COORD_T r, COORD_T g, COORD_T b)
{
	c->r = r;
	c->g = g;
	c->b = b;

	return c;
}

color_t *color_add(color_t *dest, color_t *a, color_t *b)
{
	dest->r = a->r + b->r;
	dest->g = a->g + b->g;
	dest->b = a->b + b->b;

	return dest;
}

color_t *color_scale(color_t *dest, color_t *a, COORD_T b)
{
	dest->r = a->r * b;
	dest->g = a->g * b;
	dest->b = a->b * b;

	return dest;
}

color_t *color_scale_vector(color_t *dest, color_t *a, vector_t *v)
{
	dest->r = a->r * v->x;
	dest->g = a->g * v->y;
	dest->b = a->b * v->z;

	return dest;
}
