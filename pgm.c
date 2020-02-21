#include "pgm.h"

#include <stdio.h>
#include <stdlib.h>

int pgm_write_header(FILE *fp, unsigned int w, unsigned int h)
{
	return fprintf(fp, "P3\n%d %d\n255\n", w, h);
}

int pgm_write_pixel(FILE *fp, color_t *c)
{
	return fprintf(fp, "%d %d %d\n", c->r, c->g, c->b);
}

color_t *color_set(color_t *c, uint8_t r, uint8_t g, uint8_t b)
{
	if (!c) {
		c = (color_t *) malloc(sizeof(c));
	}

	c->r = r;
	c->g = g;
	c->b = b;

	return c;
}
