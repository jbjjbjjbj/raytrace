#include "viewpoint.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static const vector_t up = {0, 1, 0};

void viewpoint_init(viewpoint_t *view)
{
	// Vector from E to T
	vector_t t;
	vector_sub(&t, &view->target, &view->position);

	// Calculate orthogonal vectors to E
	vector_t b, v;
	vector_cross(&b, (vector_t *)&up, &t);

	// Convert to unit-vectors
	vector_scale_inv(&t, &t, vector_len(&t));
	vector_scale_inv(&b, &b, vector_len(&b));

	vector_cross(&v, &t, &b);

	// Calculate size of viewplane
	COORD_T gx = tanf( FOV / 2 );
	COORD_T gy = gx * ((COORD_T)view->height / view->width);

	// Calculate viewplane vectors
	vector_scale(&view->qx, &b, ( 2 * gx ) / (view->width - 1));
	vector_scale(&view->qy, &v, ( 2 * gy ) / (view->height - 1));
	
	// Viewplane starting point
	vector_copy(&view->blc, &t);
	// This will thrash b and v
	vector_sub(&view->blc, &view->blc, vector_scale(&b, &b, gx));
	vector_sub(&view->blc, &view->blc, vector_scale(&v, &v, gy));

	// Debug print
	printf("Calculated the following viewpoint stuff\n");
	printf("qx"); vector_print(&view->qx);
	printf("qy"); vector_print(&view->qy);
	printf("blc"); vector_print(&view->blc);
}

// Calculate ray for viewport w, h
// https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
void viewpoint_ray(viewpoint_t *view, vector_t *r, unsigned x, unsigned y)
{
	// Calculate ray vector
	vector_copy(r, &view->blc);

	vector_t t;
	vector_scale(&t, &view->qx, x);
	vector_add(r, r, &t);

	vector_scale(&t, &view->qy,	y);
	vector_add(r, r, &t);

	// Calculate unit vector
	vector_scale_inv(r, r, vector_len(r));
}

